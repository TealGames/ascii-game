#include "pch.hpp"
#include <variant>
#include <type_traits>
#include "AnimatorSystem.hpp"
#include "AnimatorData.hpp"
#include "HelperFunctions.hpp"
#include "Engine.hpp"

#ifdef ENABLE_PROFILER
#include "ProfilerTimer.hpp"
#endif 

namespace ECS
{
	AnimatorSystem::AnimatorSystem(Core::Engine& engine) : m_Engine(engine) {}

	void AnimatorSystem::SystemUpdate(Scene& scene, const float& deltaTime)
	{
#ifdef ENABLE_PROFILER
		ProfilerTimer timer("AnimatorSystem::SystemUpdate");
#endif 

		scene.OperateOnComponents<AnimatorData>(
			[this, &scene, &deltaTime](AnimatorData& data, ECS::Entity& entity)-> void
			{
				if (data.m_NormalizedTime >= data.GetTimeLength() && !data.GetDoLoop()) return;

				data.m_NormalizedTime += deltaTime;
				if (data.m_NormalizedTime >= data.GetTimeLength() && data.GetDoLoop())
				{
					data.m_NormalizedTime -= data.GetTimeLength();
				}

				for (auto& property : data.m_Properties)
				{
					std::visit([this, &property, &data, &entity](auto&& value) -> void
						{
							using PropertyConvertedType = std::remove_reference_t<decltype(value)>;
							using ExtractedType = AnimationPropertyType<PropertyConvertedType>::Type;
							/*Log(LogType::Warning, std::format("Trying to run animator with property variant value; {} integral: {}", 
								typeid(PropertyConvertedType).name(), typeid(ExtractedType).name()));*/

							AnimationProperty<ExtractedType>* maybeProperty = std::get_if<AnimationProperty<ExtractedType>>(&property);
							if (!Assert(this, maybeProperty != nullptr, std::format("Tried to get the current type in variant as: {} "
								"but it failed to be converted from value: {}", typeid(PropertyConvertedType).name(),
								typeid(maybeProperty).name()))) return;

							if (!Assert(this, IsValidKeyframeIndex<ExtractedType>(*maybeProperty, maybeProperty->m_KeyframeIndex),
								std::format("Tried to get next key from from index: {} but it is not valid", std::to_string(maybeProperty->m_KeyframeIndex))))
								return;

							if (data.m_NormalizedTime >= maybeProperty->m_Keyframes[maybeProperty->m_KeyframeIndex].GetTime())
							{
								std::optional<size_t> newIndex = TryGetKeyFrameAtTime<ExtractedType>(data, *maybeProperty, data.m_NormalizedTime);
								if (!Assert(this, newIndex.has_value(), std::format("Tried to get new key frame index with time: {} "
									"and end time: {} on entity: {} but failed!", std::to_string(data.m_NormalizedTime),
									std::to_string(data.GetTimeLength()), entity.GetName())))
									return;

								maybeProperty->m_KeyframeIndex = newIndex.value();
							}
							AnimationPropertyKeyframe<ExtractedType>& currentFrame = maybeProperty->m_Keyframes[maybeProperty->m_KeyframeIndex];
							AnimationPropertyKeyframe<ExtractedType>& nextFrame = GetNextKeyFrameAtIndex<ExtractedType>(*maybeProperty, maybeProperty->m_KeyframeIndex);
							float lerpVal = (data.m_NormalizedTime - currentFrame.GetTime()) / (nextFrame.GetTime() - currentFrame.GetTime());

							if (std::is_same_v<ExtractedType, int> || std::is_same_v<ExtractedType, float> || std::is_same_v<ExtractedType, std::uint8_t>)
							{
								maybeProperty->m_ComponentPropertyRef = static_cast<ExtractedType>(std::lerp(static_cast<double>(currentFrame.GetValue()),
									static_cast<double>(nextFrame.GetValue()), lerpVal));
								maybeProperty->m_ComponentDataMutationFlagRef = true;
							}
							else
							{
								LogError(this, std::format("Tried to update property in animator for entity:{} "
									"but could not find any Type specific actions to take for it (probably due to not defining actions for this type)", entity.GetName()));
							}
						}, property);
				}
			});	
	}
}

