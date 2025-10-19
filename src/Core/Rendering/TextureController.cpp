#include "Core/Rendering/TextureController.hpp"

namespace Rendering
{
	TextureSlotData::TextureSlotData() 
		: m_Type(TextureType::Texture), m_ResourcePtr(nullptr), m_ResourceId(INVALID_OBJ_ID) {}
	TextureSlotData::TextureSlotData(Texture& texture) 
		: m_Type(TextureType::Texture), m_ResourcePtr(&texture), m_ResourceId(texture.GetId()) {}
	TextureSlotData::TextureSlotData(TextureCube& texture) 
		: m_Type(TextureType::TextureCube), m_ResourcePtr(&texture), m_ResourceId(texture.GetId()) {}

	bool TextureSlotData::HasResource() const { return m_ResourcePtr != nullptr && m_ResourceId != INVALID_OBJ_ID; }
	void TextureSlotData::RemoveResource() 
	{ 
		m_ResourcePtr = nullptr; 
		m_ResourceId = INVALID_OBJ_ID;
	}
	std::string TextureSlotData::ToString() const
	{
		std::string slotsStr = " [";
		if (HasResource())
		{
			if (m_Type == TextureType::Texture)
				slotsStr += "Texture (ID: " + std::to_string(((Texture*)m_ResourcePtr)->GetId()) + ")";
			else if (m_Type == TextureType::TextureCube)
				slotsStr += "TextureCube (ID: " + std::to_string(((TextureCube*)m_ResourcePtr)->GetId()) + ")";
			else slotsStr += "UnknownType";
		}
		else slotsStr += "FREE";

		slotsStr += "]";
		return slotsStr;
	}

	TextureSlotController::TextureSlotController(const TextureControllerCallbacks& callbacks) 
		: m_slotController(), m_callbacks(callbacks) {}

	bool TextureSlotController::TryRemoveFromSlot(const USlotIndex slot)
	{
		if (slot >= MAX_TEXTURE_SLOTS)
			return false;

		if (!m_slotController[slot].HasResource())
			return false;

		const RenderObjectId id = m_slotController[slot].m_ResourceId;
		m_slotController.UnbindAnyResourceFromSlot(slot);
		m_callbacks.m_SetBindStatusFunc(id, slot, false);
		return true;
	}
	void TextureSlotController::RemoveFromSlots(const std::vector<SlotIndex>& indices)
	{
		for (size_t i = 0; i < indices.size(); i++)
		{
			if (indices[i] < 0 || indices[i] >= MAX_TEXTURE_SLOTS)
				continue;

			TryRemoveFromSlot(indices[i]);
		}
	}
	
	std::string TextureSlotController::ToString() const
	{
		return m_slotController.ToString();
	}

	ImageSlotData::ImageSlotData() : m_ResourcePtr(nullptr) {}
	ImageSlotData::ImageSlotData(Texture& texture) : m_ResourcePtr(&texture) {}
	bool ImageSlotData::HasResource() const { return m_ResourcePtr != nullptr; }
	void ImageSlotData::RemoveResource() { m_ResourcePtr = nullptr; }
	std::string ImageSlotData::ToString() const
	{
		std::string slotsStr = " [";
		if (HasResource())
		{
			slotsStr += "Texture (ID: " + std::to_string(((Texture*)m_ResourcePtr)->GetId()) + ")";
		}
		else slotsStr += "FREE";

		slotsStr += "]";
		return slotsStr;
	}

	ImageSlotController::ImageSlotController(const ImageControllerCallbacks& callbacks)
		: m_slotController(), m_callbacks(callbacks) {}

	bool ImageSlotController::TryRemoveFromSlot(const USlotIndex slot)
	{
		if (slot >= MAX_TEXTURE_SLOTS)
			return false;

		if (!m_slotController[slot].HasResource())
			return false;

		//NOTE: this only works if only textures are stored in slots
		Texture* texture = m_slotController.UnbindResourceFromSlot<Texture>(slot);
		//NOTE: only id and slot matters as args
		m_callbacks.m_SetBindStatusFunc(texture->GetId(), TexelStorageType::R8, slot, false, Rendering::AccessPermissions::Read);
		return true;
	}

	void ImageSlotController::RemoveFromSlots(const std::vector<SlotIndex>& indices)
	{
		for (size_t i = 0; i < indices.size(); i++)
		{
			if (indices[i] < 0 || indices[i] >= MAX_TEXTURE_SLOTS)
				continue;

			TryRemoveFromSlot(indices[i]);
		}
	}

	std::string ImageSlotController::ToString() const
	{
		return m_slotController.ToString();
	}
}