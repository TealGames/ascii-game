#pragma once
#include <optional>
#include "MultiBodySystem.hpp"
#include "AnimatorData.hpp"

namespace Core
{
	class Engine;
}
namespace ECS
{
	class AnimatorSystem : MultiBodySystem
	{
	private:
		Core::Engine& m_Engine;

	public:

	private:
		bool IsValidKeyframeIndex(const AnimatorData& data, const size_t& index) const;

		std::optional<size_t> TryGetKeyFrameAtTime(const AnimatorData& data, const float& time) const;
		AnimationKeyframe& GetNextKeyFrameAtIndex(AnimatorData& data, const size_t& index) const;

		AnimationProperty* TryFindMatchingProperty(AnimationKeyframe& keyframe, const AnimationProperty& property);

	public:
		AnimatorSystem(Core::Engine& engine);
		void SystemUpdate(Scene& scene, const float& deltaTime) override;

	};
}

