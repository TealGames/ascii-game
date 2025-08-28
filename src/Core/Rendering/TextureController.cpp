#include "Core/Rendering/TextureController.hpp"

namespace Rendering
{
	bool TextureSlotData::HasResource() const { return m_ResourcePtr != nullptr; }
	void TextureSlotData::RemoveResource() { m_ResourcePtr = nullptr; }

	TextureController::TextureController() : m_textureSlots(), m_nextAvailableIndex(0) {}

	/*Texture* TextureController::UnbindTextureFromSlot(const TextureSlotIndex slot)
	{
		return UnbindResourceFromSlot<Texture*>()
	}
	TextureCube* TextureController::UnbindTextureCubeFromSlot(const TextureSlotIndex slot)
	{
		m_textureSlots[slot]->UnbindFromSlot();
		Texture* texture = m_textureSlots[slot];
		m_textureSlots[slot].RemoveResource();

		return texture;
	}*/
	bool TextureController::ValidAvailableIndexCheck() const
	{
		if (m_nextAvailableIndex >= MAX_TEXTURE_SLOTS || m_nextAvailableIndex == INVALID_TEXTURE_SLOT_INDEX
			|| m_nextAvailableIndex < 0)
		{
			LogError(std::format("Current available index:{} is invalid. Max slots:{}", 
				m_nextAvailableIndex, MAX_TEXTURE_SLOTS));
			return false;
		}
		return true;
	}

	void TextureController::UnbindAnyResourceFromSlot(const TextureSlotIndex slot)
	{
		const TextureType type = m_textureSlots[slot].m_Type;
		if (type == TextureType::Texture) UnbindResourceFromSlot<Texture>(slot);
		else if (type== TextureType::TextureCube) UnbindResourceFromSlot<TextureCube>(slot);
		else
		{
			LogError(std::format("Attempted to unbind resource in slot:{} "
				"to slot but it has no type defined in enum", slot));
		}
	}

	void TextureController::FindNextAvailableIndex(const TextureSlotIndex initialIndex)
	{
		m_nextAvailableIndex = INVALID_TEXTURE_SLOT_INDEX;
	
		for (TextureSlotIndex i = initialIndex + 1; i < MAX_TEXTURE_SLOTS; i++)
		{
			if (!m_textureSlots[i].HasResource())
			{
				m_nextAvailableIndex = i;
				break;
			}
		}
	}

	TextureSlotIndex TextureController::TryAddTextureToAvailableSlot(Texture* texture)
	{
		if (!ValidAvailableIndexCheck())
			return INVALID_TEXTURE_SLOT_INDEX;

		TextureSlotIndex selectedSlot = m_nextAvailableIndex;
		BindResourceToSlot<Texture>(selectedSlot, texture);
		//Since we always seek to get lowest available index, if we have have used the last slot
		//we know there are none left earlier
		if (m_nextAvailableIndex != MAX_TEXTURE_SLOTS - 1)
			FindNextAvailableIndex(selectedSlot);
		
		return selectedSlot;
	}
	std::vector<TextureSlotIndex> TextureController::TryAddTexturesToAvailableSlots(Texture textures[], const size_t size)
	{
		return TryAddResourceToAvailableSlots<Texture>(textures, size);
	}
	std::vector<TextureSlotIndex> TextureController::TryAddTextureCubesToAvailableSlots(TextureCube cubes[], const size_t size)
	{
		return TryAddResourceToAvailableSlots<TextureCube>(cubes, size);
	}
	/*
	TextureSlotIndex TextureController::TryRemoveTextureFromSlot(Texture* texture)
	{
		for (TextureSlotIndex i = 0; i < MAX_TEXTURE_SLOTS; i++)
		{
			if (m_textureSlots[i].m_ResourcePtr == texture)
			{
				UnbindTextureFromSlot(i);
				if (i < m_nextAvailableIndex)
					m_nextAvailableIndex = i;

				return i;
			}
		}
		return INVALID_TEXTURE_SLOT_INDEX;
	}
	Texture* TextureController::TryRemoveTextureFromSlot(const TextureSlotIndex slot)
	{
		if (slot >= MAX_TEXTURE_SLOTS)
			return nullptr;

		Texture* unboundTex= UnbindTextureFromSlot(slot);
		if (slot < m_nextAvailableIndex)
			m_nextAvailableIndex = slot;

		return unboundTex;
	}
	*/
	void TextureController::RemoveFromSlots(const TextureSlotIndex startIndex, const size_t size)
	{
		if (startIndex >= MAX_TEXTURE_SLOTS)
			return;

		for (TextureSlotIndex i = startIndex; i < startIndex + size; i++)
		{
			UnbindAnyResourceFromSlot(i);
		}
		if (startIndex < m_nextAvailableIndex)
			m_nextAvailableIndex = startIndex;
	}
	void TextureController::TryRemoveFromSlot(TextureSlotIndex slot)
	{
		if (slot >= MAX_TEXTURE_SLOTS)
			return;

		UnbindAnyResourceFromSlot(slot);
		if (slot < m_nextAvailableIndex)
			m_nextAvailableIndex = slot;
	}

	bool TextureController::HasTextureInSlot(const TextureSlotIndex slot) const
	{
		if (slot >= MAX_TEXTURE_SLOTS)
			return false;

		return m_textureSlots[slot].HasResource();
	}

	void TextureController::ClearAllSlots()
	{
		for (TextureSlotIndex i = 0; i < MAX_TEXTURE_SLOTS; i++)
		{
			if (m_textureSlots[i].HasResource())
				UnbindAnyResourceFromSlot(i);
		}
		m_nextAvailableIndex = 0;
	}
	std::string TextureController::ToString() const
	{
		std::string slotsStr = "";
		for (const auto& slot : m_textureSlots)
		{
			slotsStr += " [";
			if (slot.HasResource())
			{
				slotsStr += "USED: " + (slot.m_Type == TextureType::Texture) ? "Texture" : "TextureCube";
			}
			else
			{
				slotsStr += "FREE";
			}
			slotsStr += "]";
		}
		return std::format("[TextureController nextIndex:{} Slots:{}]", m_nextAvailableIndex, slotsStr);
	}
}