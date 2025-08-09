#pragma once
#include <array>
#include "Core/Rendering/Texture.hpp"

namespace Rendering
{
	constexpr TextureSlotIndex MAX_TEXTURE_SLOTS = 16;
	class TextureController
	{
	private:
		TextureSlotIndex m_nextAvailableIndex;
		std::array<Texture*, MAX_TEXTURE_SLOTS> m_textureSlots;
	public:

	private:
		void BindTextureToSlot(const TextureSlotIndex slot, Texture* texture);
		Texture* UnbindTextureFromSlot(const TextureSlotIndex slot);
	public:
		TextureController();

		TextureSlotIndex AddTextureToAvailableSlot(Texture* texture);
		TextureSlotIndex RemoveTextureFromSlot(Texture* texture);
		Texture* RemoveTextureFromSlot(TextureSlotIndex slot);

		bool HasTextureInSlot(const TextureSlotIndex slot) const;

		void ClearAllSlots();
	};
}