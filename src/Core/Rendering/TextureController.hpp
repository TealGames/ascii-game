#pragma once
#include <array>
#include <type_traits>
#include "Core/Rendering/Texture.hpp"
#include "Core/Analyzation/Debug.hpp"

namespace Rendering
{
	struct TextureSlotData
	{
		TextureType m_Type = TextureType::Texture;
		void* m_ResourcePtr = nullptr;

		bool HasResource() const;
		void RemoveResource();
	};

	constexpr TextureSlotIndex MAX_TEXTURE_SLOTS = 16;
	class TextureController
	{
	private:
		TextureSlotIndex m_nextAvailableIndex;
		std::array<TextureSlotData, MAX_TEXTURE_SLOTS> m_textureSlots;
	public:

	private:
		bool ValidAvailableIndexCheck() const;

		template<typename T>
		void BindResourceToSlot(const TextureSlotIndex slot, T* resourcePtr)
		{
			resourcePtr->BindToSlot(slot);
			TextureType type = TextureType::Texture;
			if constexpr (std::is_same_v<T, Texture>) type = TextureType::Texture;
			else if constexpr (std::is_same_v<T, TextureCube>) type = TextureType::TextureCube;
			else
			{
				LogError(std::format("Attempted to bind resource of type:{} "
					"to slot but it has no type defined in enum", typeid(T).name()));
			}
			m_textureSlots[slot] = TextureSlotData{ type, resourcePtr };
		}

		void UnbindAnyResourceFromSlot(const TextureSlotIndex slot);

		template<typename T>
		T* UnbindResourceFromSlot(const TextureSlotIndex slot)
		{
			T* tPtr = static_cast<T*>(m_textureSlots[slot].m_ResourcePtr);
			tPtr->UnbindFromSlot();
			m_textureSlots[slot].RemoveResource();

			return tPtr;
		}
		/*Texture* UnbindTextureFromSlot(const TextureSlotIndex slot);
		TextureCube* UnbindTextureCubeFromSlot(const TextureSlotIndex slot);*/
		void FindNextAvailableIndex(const TextureSlotIndex initialIndex);

		template<typename T>
		std::vector<TextureSlotIndex> TryAddResourceToAvailableSlots(T textures[], const size_t size)
		{
			if (!ValidAvailableIndexCheck())
				return {};

			if (size == 0)
			{
				LogError(std::format("Attempted to add textures to available slots with size 0"));
				return {};
			}

			std::vector<TextureSlotIndex> slots = {};
			if (m_nextAvailableIndex + size >= MAX_TEXTURE_SLOTS || m_nextAvailableIndex == INVALID_TEXTURE_SLOT_INDEX
				|| m_nextAvailableIndex < 0)
			{
				LogError(std::format("Attempted to add textures({}) to next available slot indices "
					"but there are not enough free spaces left. Total size:{}", size, MAX_TEXTURE_SLOTS));
				return slots;
			}
			for (size_t i = 0; i < size; i++)
			{
				BindResourceToSlot<T>(m_nextAvailableIndex, &textures[i]);
				slots.push_back(m_nextAvailableIndex);
				if (i < size - 1) m_nextAvailableIndex++;
			}
			if (m_nextAvailableIndex != MAX_TEXTURE_SLOTS - 1)
				FindNextAvailableIndex(m_nextAvailableIndex);

			return slots;
		}
	public:
		TextureController();

		TextureSlotIndex TryAddTextureToAvailableSlot(Texture* texture);
		std::vector<TextureSlotIndex> TryAddTexturesToAvailableSlots(Texture textures[], const size_t size);
		std::vector<TextureSlotIndex> TryAddTextureCubesToAvailableSlots(TextureCube cubes[], const size_t size);

		//TextureSlotIndex TryRemoveTextureFromSlot(Texture* texture);
		//Texture* TryRemoveTextureFromSlot(TextureSlotIndex slot);
		void TryRemoveFromSlot(TextureSlotIndex index);
		void RemoveFromSlots(const TextureSlotIndex startIndex, const size_t size);

		bool HasTextureInSlot(const TextureSlotIndex slot) const;

		void ClearAllSlots();
		std::string ToString() const;
	};
}