#pragma once
#include <optional>
#include <type_traits>
#include "MultiBodySystem.hpp"
#include "AnimatorData.hpp"
#include "Debug.hpp"

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
		template<typename T>
		bool IsValidKeyframeIndex(const AnimationProperty<T>& property, const size_t& index) const
		{
			return 0 <= index && index < property.m_Keyframes.size();
		}

		template<typename T>
		std::optional<size_t> TryGetKeyFrameAtTime(const AnimatorData& data, const AnimationProperty<T>& property, const float& time) const
		{
			if (property.m_Keyframes.empty()) return std::nullopt;
			if (property.m_Keyframes.size() == 1)
			{
				if (property.m_Keyframes.front().GetTime() == time) return 0;
				else return std::nullopt;
			}
			float firstTime = property.m_Keyframes.front().GetTime();
			if (time < firstTime) return std::nullopt;
			if (Utils::ApproximateEqualsF(time, firstTime)) return 0;

			if (time > data.GetEndTime()) return std::nullopt;
			if (Utils::ApproximateEqualsF(data.GetEndTime(), firstTime)) return property.m_Keyframes.size() - 1;

			int left = 1;
			int right = property.m_Keyframes.size() - 1;
			int middle = 0;
			while (left <= right && left >= 1 && right < property.m_Keyframes.size())
			{
				middle = (right - left) / 2 + left;
				if (Utils::ApproximateEqualsF(property.m_Keyframes[middle].GetTime(), time))
					return middle;

				if (middle != 0 && property.m_Keyframes[middle - 1].GetTime() <= time &&
					time <= property.m_Keyframes[middle].GetTime())
				{
					return middle - 1;
				}
				else if (property.m_Keyframes[middle].GetTime() < time)
				{
					left = middle + 1;
				}
				else if (property.m_Keyframes[middle].GetTime() > time)
				{
					right = middle - 1;
				}
			}
			return std::nullopt;
		}

		//TODO: maybe optimize this by returning ref instead of pointer and having valid index as a precondition
		template<typename T>
		AnimationPropertyKeyframe<T>& GetNextKeyFrameAtIndex(AnimationProperty<T>& property, const size_t& index) const
		{
			size_t nextKeyframeIndex = index + 1 % property.m_Keyframes.size();
			return property.m_Keyframes[nextKeyframeIndex];
		}

		/*AnimationProperty* AnimatorSystem::TryFindMatchingProperty(AnimationKeyframe& keyframe, const AnimationProperty& property)
		{
			auto result = keyframe.m_Properties.find(property.m_PropertyName);
			if (result == keyframe.m_Properties.end()) return nullptr;
			else return &(result->second);
		}*/

		template<typename T>
		void PropertyActions(const ECS::Entity& entity, const AnimatorData& data, AnimationProperty<T>& property)
		{
			if (data.m_NormalizedTime >= property.m_Keyframes[property.m_KeyframeIndex].GetTime())
			{
				std::optional<size_t> newIndex = TryGetKeyFrameAtTime<T>(data, property, data.m_NormalizedTime);
				if (!Assert(this, newIndex.has_value(), std::format("Tried to get new key frame index with time: {} "
					"and end time: {} on entity: {} but failed!", std::to_string(data.m_NormalizedTime),
					std::to_string(data.m_EndTime), entity.m_Name)))
					return;

				property.m_KeyframeIndex = newIndex.value();
			}

			AnimationPropertyKeyframe<T>& currentFrame = property.m_Keyframes[property.m_KeyframeIndex];
			AnimationPropertyKeyframe<T>& nextFrame = GetNextKeyFrameAtIndex<T>(property, property.m_KeyframeIndex);
			float lerpVal = (data.m_NormalizedTime - currentFrame.GetTime()) / (nextFrame.GetTime() - currentFrame.GetTime());

			if (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, std::uint8_t>)
			{
				property.m_ComponentPropertyRef = static_cast<T>(std::lerp(static_cast<double>(currentFrame.GetValue()), 
																		   static_cast<double>(nextFrame.GetValue()), lerpVal));
				property.m_ComponentDataMutationFlagRef = true;
				//Log(LogType::Warning, std::format("Set property value to; {}", std::to_string(property.m_ComponentDataMutationFlagRef)));
			}
			else
			{
				LogError(this, std::format("Tried to update property in animator for entity:{} "
					"but could not find any Type specific actions to take for it (probably due to not defining actions for this type)", entity.m_Name));
			}
		}

	public:
		AnimatorSystem(Core::Engine& engine);
		void SystemUpdate(Scene& scene, const float& deltaTime) override;

	};
}

