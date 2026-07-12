#pragma once
#include <optional>
#include <type_traits>
#include "ECS/Component/Types/World/AnimatorComponent.hpp"
#include "Utils/Debug.hpp"

namespace Engine::Scenes { class Scene; }
namespace Engine::Camera { class CameraComponent; }

namespace Engine::Animation
{
	class AnimatorSystem
	{
	private:

	public:

	private:
		template<typename T>
		bool IsValidKeyframeIndex(const AnimationProperty<T>& property, const size_t& index) const
		{
			return 0 <= index && index < property.m_Keyframes.size();
		}

		template<typename T>
		std::optional<size_t> TryGetKeyFrameAtTime(const AnimatorComponent& data, const AnimationProperty<T>& property, const float& time) const
		{
			if (property.m_Keyframes.empty()) return std::nullopt;
			if (property.m_Keyframes.size() == 1)
			{
				if (property.m_Keyframes.front().GetTime() == time) return 0;
				else return std::nullopt;
			}
			float firstTime = property.m_Keyframes.front().GetTime();
			if (time < firstTime) return std::nullopt;
			if (::Math::ApproximateEqualsF(time, firstTime)) return 0;

			if (time > data.GetDefaultTimeLength()) return std::nullopt;
			if (::Math::ApproximateEqualsF(data.GetDefaultTimeLength(), firstTime)) return property.m_Keyframes.size() - 1;

			int left = 1;
			int right = property.m_Keyframes.size() - 1;
			int middle = 0;
			while (left <= right && left >= 1 && right < property.m_Keyframes.size())
			{
				middle = (right - left) / 2 + left;
				if (::Math::ApproximateEqualsF(property.m_Keyframes[middle].GetTime(), time))
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
			size_t nextKeyframeIndex = (index + 1) % property.m_Keyframes.size();
			try
			{
				return property.m_Keyframes.at(nextKeyframeIndex);
			}
			catch (const std::exception& e)
			{
				LogError(std::format("Tried to use index:{} ({}) of keyframes:{}", nextKeyframeIndex, index+1, property.m_Keyframes.size()));
				throw std::invalid_argument("Invalid next key frame call");
			}
		}

	public:
		AnimatorSystem();
		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};
}

