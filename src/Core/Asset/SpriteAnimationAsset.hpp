#pragma once
#include "Core/Asset/Asset.hpp"
#include "Core/Visual/SpriteAnimation.hpp"

namespace Engine::Animation
{
	class SpriteAnimationAsset : public Assets::Asset
	{
	private:
		SpriteAnimation m_animation;
	public:
		static const std::array<std::string_view,1> EXTENSIONS;

	protected:
	public:
		SpriteAnimationAsset(const std::filesystem::path& path);

		const SpriteAnimation& GetAnimation() const;
		SpriteAnimation& GetAnimationMutable();

		void UpdateAssetFromFile() override;
		void SaveToPath(const std::filesystem::path& path) override;

		std::string ToString() const override;
	};
}


