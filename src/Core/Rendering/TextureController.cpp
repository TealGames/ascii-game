#include "Core/Rendering/TextureController.hpp"
#include "Core/Analyzation/Debug.hpp"

namespace Rendering
{
	TextureController::TextureController() : m_textureSlots(), m_nextAvailableIndex(0) {}

	void TextureController::BindTextureToSlot(const TextureSlotIndex slot, Texture* texture)
	{
		texture->BindToSlot(slot);
		m_textureSlots[slot] = texture;
	}
	Texture* TextureController::UnbindTextureFromSlot(const TextureSlotIndex slot)
	{
		m_textureSlots[slot]->UnbindFromSlot();
		Texture* texture = m_textureSlots[slot];
		m_textureSlots[slot] = nullptr;

		return texture;
	}

	TextureSlotIndex TextureController::AddTextureToAvailableSlot(Texture* texture)
	{
		if (m_nextAvailableIndex >= MAX_TEXTURE_SLOTS || m_nextAvailableIndex== INVALID_TEXTURE_SLOT_INDEX 
			|| m_nextAvailableIndex<0)
		{
			LogError(std::format("Attempted to add texture:{} to next available slot index "
				"but there are no free spaces left. Total size:{}", texture->ToString(), MAX_TEXTURE_SLOTS));
			return INVALID_TEXTURE_SLOT_INDEX;
		}
		TextureSlotIndex selectedSlot = m_nextAvailableIndex;
		BindTextureToSlot(selectedSlot, texture);
		
		m_nextAvailableIndex = INVALID_TEXTURE_SLOT_INDEX;
		//Since we always seek to get lowest available index, if we have have used the last slot
		//we know there are none left earlier
		if (m_nextAvailableIndex == MAX_TEXTURE_SLOTS - 1)
			return selectedSlot;

		for (TextureSlotIndex i = selectedSlot+1; i < MAX_TEXTURE_SLOTS; i++)
		{
			if (m_textureSlots[i] == nullptr)
			{
				m_nextAvailableIndex = i;
				break;
			}
		}
		return selectedSlot;
	}
	TextureSlotIndex TextureController::RemoveTextureFromSlot(Texture* texture)
	{
		for (TextureSlotIndex i = 0; i < MAX_TEXTURE_SLOTS; i++)
		{
			if (m_textureSlots[i] == texture)
			{
				UnbindTextureFromSlot(i);
				if (i < m_nextAvailableIndex)
					m_nextAvailableIndex = i;

				return i;
			}
		}
		return INVALID_TEXTURE_SLOT_INDEX;
	}
	Texture* TextureController::RemoveTextureFromSlot(const TextureSlotIndex slot)
	{
		if (slot >= MAX_TEXTURE_SLOTS)
			return nullptr;

		Texture* unboundTex= UnbindTextureFromSlot(slot);
		if (slot < m_nextAvailableIndex)
			m_nextAvailableIndex = slot;

		return unboundTex;
	}

	bool TextureController::HasTextureInSlot(const TextureSlotIndex slot) const
	{
		if (slot >= MAX_TEXTURE_SLOTS)
			return false;

		return m_textureSlots[slot] != nullptr;
	}

	void TextureController::ClearAllSlots()
	{
		for (TextureSlotIndex i = 0; i < MAX_TEXTURE_SLOTS; i++)
		{
			if (m_textureSlots[i] != nullptr)
				UnbindTextureFromSlot(i);
		}
		m_nextAvailableIndex = 0;
	}
}