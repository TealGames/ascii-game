#pragma once
#include <array>
#include <unordered_map>
#include <type_traits>
#include <functional>
#include "Core/Rendering/Texture.hpp"
#include "Utils/Debug.hpp"
#include "Utils/DataStructure/ResourceSlotController.hpp"
#include "Utils/TemplateConcepts.hpp"

namespace Engine::Rendering
{
	DEFINE_TEMPLATE_HAS_FUNCTION(GetId, RenderObjectId);
	DEFINE_TEMPLATE_HAS_FUNCTION(GetStorageType, TexelStorageType);

	template<typename T>
	concept IsValidResource = HasFunctionGetStorageType<T> && HasFunctionGetId<T>;

	DEFINE_TEMPLATE_HAS_PROPERTY(m_ResourcePtr, void*);
	DEFINE_TEMPLATE_HAS_PROPERTY(m_ResourceId, RenderObjectId);
	DEFINE_TEMPLATE_HAS_VOID_FUNCTION(RemoveResource);
	DEFINE_TEMPLATE_HAS_FUNCTION(HasResource, bool);

	template<typename TSlotData, std::size_t MAX_SLOTS>
	requires (HasPropertym_ResourcePtr<TSlotData> && 
			  HasFunctionHasResource<TSlotData> && 
		      HasVoidFunctionRemoveResource<TSlotData> && 
		      ::Utils::HasFunctionToString<TSlotData>)
	class GpuResourceSlotController
	{
	private:
		ResourceSlotController<TSlotData, MAX_SLOTS> m_slotController;
		std::unordered_map<RenderObjectId, USlotIndex> m_resourceSlotMap;
	public:

	private:
	public:
		GpuResourceSlotController() : m_slotController(), m_resourceSlotMap() {}

		SlotIndex TryGetResourceSlot(const RenderObjectId id) const
		{
			ENGINE_ASSERT(id != INVALID_OBJ_ID, "Attempted to get invalid ID resource from GPU resource slot");

			if (m_resourceSlotMap.empty())
				return INVALID_SLOT_INDEX;

			auto it = m_resourceSlotMap.find(id);
			if (it != m_resourceSlotMap.end())
				return it->second;

			return INVALID_SLOT_INDEX;
		}

		template<typename TResource>
		requires HasFunctionGetId<TResource> && std::constructible_from<TSlotData, TResource&>
		SlotIndex BindResourceToFreeSlot(TResource& resource)
		{
			const SlotIndex slotAdded = m_slotController.TryAdd(TSlotData(resource));
			const RenderObjectId objectId = resource.GetId();
			ENGINE_ASSERT(objectId != INVALID_OBJ_ID, "Attempted to bind invalid ID resource to GPU resource slot");
			ENGINE_ASSERT(slotAdded != INVALID_SLOT_INDEX, "Attempted to bind resource to GPU resource slot "
				"but failed to add to controller: {}", ToString());

			m_resourceSlotMap.emplace(objectId, slotAdded);
			return slotAdded;
		}

		template<typename TResource>
		requires HasFunctionGetId<TResource> && std::constructible_from<TSlotData, TResource&>
		TSlotData RebindResourceInSlot(const USlotIndex slot, TResource& resource)
		{
			ENGINE_ASSERT(slot < MAX_SLOTS, "Attempted to replace resource from out of bounds "
				"[0,{}) slot: {}", MAX_SLOTS, slot);
			ENGINE_ASSERT(!m_slotController[slot].HasResource(), 
				"Attempted to replace resource at slot: {} but there is no resource there. "
				"Use BindResourceToFreeSlot instead", slot);

			const RenderObjectId objectId = resource.GetId();
			ENGINE_ASSERT(objectId != INVALID_OBJ_ID, "Attempted to rebind resource at slot:{} with invalid ID resource", slot);

			const TSlotData oldData= m_slotController.ReplaceAt(slot, TSlotData(resource));
			m_resourceSlotMap[objectId] = slot;
			return oldData;
		}

		void UnbindResourceFromSlot(const USlotIndex slot, const RenderObjectId resourceId)
		{
			ENGINE_ASSERT(resourceId != INVALID_OBJ_ID, "Attempted to unbind invalid ID resource from slot:{}", slot);
			ENGINE_ASSERT(slot < MAX_SLOTS, "Attempted to unbind resource using id from out of bounds "
				"[0,{}) slot: {}", MAX_SLOTS, slot);

			m_slotController[slot].RemoveResource();
			m_slotController.RemoveAtUnsafe(slot);
			m_resourceSlotMap.erase(resourceId);
		}
		void UnbindAnyResourceFromSlot(const USlotIndex slot)
			requires HasPropertym_ResourceId<TSlotData>
		{
			const RenderObjectId id = m_slotController[slot].m_ResourceId;
			ENGINE_ASSERT(id != INVALID_OBJ_ID, "Attempted to unbind any resource from slot: {} "
				"but that slot has invalid resource ID", slot);

			UnbindResourceFromSlot(slot, id);
		}

		template<typename TResource>
		requires HasFunctionGetId<TResource>
		TResource* UnbindResourceFromSlot(const USlotIndex slot)
		{
			ENGINE_ASSERT(slot < MAX_SLOTS, "Attempted to unbind typed resource from out of bounds "
				"[0,{}) slot: {}", MAX_SLOTS, slot);

			TResource* resourcePtr = static_cast<TResource*>(m_slotController[slot].m_ResourcePtr);
			UnbindResourceFromSlot(slot, resourcePtr->GetId());
			return resourcePtr;
		}

		bool HasResourceInSlot(const USlotIndex slot) const
		{
			ENGINE_ASSERT(slot < MAX_SLOTS, "Attempted to check resource from out of bounds "
				"[0,{}) slot: {}", MAX_SLOTS, slot);
				
			return m_slotController[slot].HasResource();
		}

		void ClearAllSlots()
			requires HasPropertym_ResourceId<TSlotData>
		{
			for (size_t i = 0; i < MAX_SLOTS; i++)
			{
				if (m_slotController[i].HasResource())
					UnbindAnyResourceFromSlot(i);
			}
		}
		std::size_t GetEmptySlotCount() const { return m_slotController.GetEmptySlotCount(); }

		TSlotData& operator[](const USlotIndex& index) { return m_slotController[index]; }
		const TSlotData& operator[](const USlotIndex& index) const { return m_slotController[index]; }

		std::string ToString() const
		{
			std::string slotsStr = "";
			for (const auto& slot : m_slotController)
			{
				slotsStr += slot.ToString();
			}
			return "[TextureController Slots:" + slotsStr + "]";
		}
	};

	struct TextureSlotData
	{
		RenderObjectId m_ResourceId;
		TextureType m_Type;
		void* m_ResourcePtr;

		TextureSlotData();
		TextureSlotData(Texture& texture);
		TextureSlotData(TextureCube& texture);

		bool HasResource() const;
		void RemoveResource();

		std::string ToString() const;
	};
	struct TextureControllerCallbacks
	{
		void(*m_SetBindStatusFunc)(const RenderObjectId, const USlotIndex index, const bool status);
	};

	constexpr USlotIndex MAX_TEXTURE_SLOTS = 16;
	class TextureSlotController
	{
	private:
		GpuResourceSlotController<TextureSlotData, MAX_TEXTURE_SLOTS> m_slotController;
		TextureControllerCallbacks m_callbacks;
	public:

	private:
	public:
		TextureSlotController(const TextureControllerCallbacks& callbacks);

		template<typename T>
		requires IsValidResource<T>
		SlotIndex TryBindToFreeSlot(T& resource)
		{
			const SlotIndex index = m_slotController.BindResourceToFreeSlot<T>(resource);
			if (index != INVALID_SLOT_INDEX) m_callbacks.m_SetBindStatusFunc(resource.GetId(), index, true);
			return index;
		}
		template<typename T>
		requires IsValidResource<T>
		std::vector<SlotIndex> TryBindToFreeSlots(T resources[], const size_t size)
		{
			ENGINE_ASSERT(m_slotController.GetEmptySlotCount() >= size, "Attempted to add resources({}) to next available slot indices "
				"but there are not enough free spaces left. Total size:{}", size, MAX_TEXTURE_SLOTS);

			std::vector<SlotIndex> slots = {};
			for (size_t i = 0; i < size; i++)
			{
				slots.push_back(TryBindToFreeSlot<T>(resources[i]));
			}
			return slots;
		}

		template<typename T>
		requires IsValidResource<T>
		TextureSlotData RebindAtSlot(const USlotIndex slot, T& resource)
		{
			TextureSlotData oldData = m_slotController.RebindResourceInSlot<T>(slot, resource);
			//NOTE: by doing this direct call rather than doing remove and bind again we same some operations
			m_callbacks.m_SetBindStatusFunc(resource.GetId(), slot, true);
			return oldData;
		}

		template<typename T>
		requires IsValidResource<T>
		SlotIndex TryGetSlot(const T& resource) const
		{
			return m_slotController.TryGetResourceSlot(resource.GetId());
		}
		bool TryRemoveFromSlot(const USlotIndex index);

		template<typename T>
		requires IsValidResource<T>
		bool TryRemoveFromSlot(const T& resource)
		{
			const RenderObjectId id = resource.GetId();
			const SlotIndex slot = m_slotController.TryGetResourceSlot(id);
			ENGINE_ASSERT(slot != INVALID_SLOT_INDEX, "Attempted to remove resource:{} from texture slot "
				"but it was not found in any slots: {}", id, ToString());

			return TryRemoveFromSlot(slot);
		}
		void RemoveFromSlots(const std::vector<SlotIndex>& indices);

		std::string ToString() const;
	};

	struct ImageSlotData
	{
		void* m_ResourcePtr = nullptr;

		ImageSlotData();
		ImageSlotData(Texture& texture);

		bool HasResource() const;
		void RemoveResource();

		std::string ToString() const;
	};
	struct ImageControllerCallbacks
	{
		void(*m_SetBindStatusFunc)(const RenderObjectId, const TexelStorageType storage, 
			const USlotIndex index, const bool status, const AccessPermissions permissions);
	};

	template<typename TFunc, typename TResource>
	concept IsImageBindFunc = ::Utils::IsInvocableType<void, TFunc, RenderObjectId, USlotIndex, bool>;
	
	constexpr USlotIndex MAX_IMAGE_SLOTS = 8;
	class ImageSlotController
	{
	private:
		GpuResourceSlotController<ImageSlotData, MAX_IMAGE_SLOTS> m_slotController;
		ImageControllerCallbacks m_callbacks;
	public:

	private:
	public:
		ImageSlotController(const ImageControllerCallbacks& callbacks);

		template<typename T>
		requires IsValidResource<T>
		SlotIndex TryBindToFreeSlot(T& resource, const AccessPermissions permissions)
		{
			const SlotIndex index = m_slotController.BindResourceToFreeSlot<T>(resource);
			if (index != INVALID_SLOT_INDEX)
			{
				m_callbacks.m_SetBindStatusFunc(resource.GetId(),
					resource.GetStorageType(), index, true, permissions);
			}
			return index;
		}
		template<typename T>
		requires IsValidResource<T>
		std::vector<SlotIndex> TryBindToFreeSlots(T resources[], const size_t size, const AccessPermissions permissions)
		{
			std::vector<SlotIndex> slots = {};
			ENGINE_ASSERT(m_slotController.GetEmptySlotCount() >= size, "Attempted to add resources({}) to next available slot indices "
				"but there are not enough free spaces left. Total size:{}", size, MAX_IMAGE_SLOTS);

			for (size_t i = 0; i < size; i++)
			{
				slots.push_back(TryBindToFreeSlot<T>(resources[i], permissions));
			}
			return slots;
		}

		template<typename T>
		requires IsValidResource<T>
		ImageSlotData RebindAtSlot(const USlotIndex slot, T& resource, const AccessPermissions permissions)
		{
			ImageSlotData oldData= m_slotController.RebindResourceInSlot<T>(slot, resource);
			//NOTE: by doing this direct call rather than doing remove and bind again we same some operations
			m_callbacks.m_SetBindStatusFunc(resource.GetId(),
				resource.GetStorageType(), slot, true, permissions);
			return oldData;
		}

		template<typename T>
		requires IsValidResource<T>
		SlotIndex TryGetSlot(const T& resource) const
		{
			return m_slotController.TryGetResourceSlot(resource.GetId());
		}
		bool TryRemoveFromSlot(const USlotIndex index);

		template<typename T>
		requires IsValidResource<T>
		bool TryRemoveFromSlot(const T& resource)
		{
			const RenderObjectId id = resource.GetId();
			const SlotIndex slot = m_slotController.TryGetResourceSlot(id);
			ENGINE_ASSERT(slot != INVALID_SLOT_INDEX, "Attempted to remove resource:{} from texture slot "
				"but it was not found in any slots: {}", id, ToString());

			return TryRemoveFromSlot(slot);
		}
		void RemoveFromSlots(const std::vector<SlotIndex>& indices);

		std::string ToString() const;
	};
}