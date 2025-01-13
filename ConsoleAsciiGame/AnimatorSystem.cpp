#include "pch.hpp"
#include <variant>
#include "AnimatorSystem.hpp"
#include "AnimatorData.hpp"
#include "HelperFunctions.hpp"
#include "Engine.hpp"

namespace ECS
{
	AnimatorSystem::AnimatorSystem(Core::Engine& engine) : m_Engine(engine) {}

	void AnimatorSystem::SystemUpdate(Scene& scene, const float& deltaTime)
	{
		scene.OperateOnComponents<AnimatorData>(
			[this, &scene, &deltaTime](AnimatorData& data, ECS::Entity& entity)-> void
			{
				if (data.m_NormalizedTime >= data.m_EndTime && !data.m_Loop) return;

				data.m_NormalizedTime += deltaTime;
				if (data.m_NormalizedTime >= data.m_EndTime && data.m_Loop)
				{
					data.m_NormalizedTime -= data.m_EndTime;
				}

				if (!Utils::Assert(this, IsValidKeyframeIndex(data, data.m_KeyframeIndex),
					std::format("Tried to get next key from from index: {} but it is not valid", std::to_string(data.m_KeyframeIndex))))
					return;

				//Ideally we would just increase the index, but in cases where we have lag (and ultimately large 
				//delta time then it might mess this up since we want to preserve that sequence based on time
				if (data.m_NormalizedTime >= GetNextKeyFrameAtIndex(data, data.m_KeyframeIndex).m_Time)
				{
					std::optional<size_t> newIndex = TryGetKeyFrameAtTime(data, data.m_NormalizedTime);
					if (!Utils::Assert(this, newIndex.has_value(), std::format("Tried to get new key frame index with time: {} "
						"and end time: {} on entity: {} but failed!", std::to_string(data.m_NormalizedTime), 
						std::to_string(data.m_EndTime), entity.m_Name))) 
						return;

					data.m_KeyframeIndex = newIndex.value();
				}

				AnimationKeyframe& currentFrame = data.m_Keyframes[data.m_KeyframeIndex];
				AnimationKeyframe& nextFrame = GetNextKeyFrameAtIndex(data, data.m_KeyframeIndex);

				float lerpVal = (data.m_NormalizedTime - currentFrame.m_Time) / (nextFrame.m_Time - currentFrame.m_Time);

				//TODO: right now the system relies on properties not having the same name across components
				//otherwise the wrong pair might be connected 
				for (auto& property : currentFrame.m_Properties)
				{
					Utils::Log(Utils::LogType::Warning, std::format("Iterating to ANIAMTOR PROPERTY: {}", property.second.m_PropertyName));
					AnimationProperty* nextProperty = TryFindMatchingProperty(nextFrame, property.second);
					if (!Utils::Assert(this, nextProperty != nullptr && nextProperty->m_ComponentType == property.second.m_ComponentType,
						std::format("Tried to update components values for entity: {} from keyframe but matching property for {} was NULL: {} OR "
							"the found property had different component types (could have same property names but different components. "
							" Component one : {} component two(ont searched) : {}",
							entity.m_Name, property.second.ToString(), nextProperty == nullptr, ToString(property.second.m_ComponentType),
							nextProperty != nullptr ? ToString(nextProperty->m_ComponentType) : "NULL"))) continue;

					//TODO: this should probably be abstracted so it can work for any type since the process is relatively the same for most integral types
					//but also some other types might have diferences so it might not be the best idea to abstract all of them
					if (int* currentVal = std::get_if<int>(&(property.second.m_Value)); 
						int* nextVal= std::get_if<int>(&(nextProperty->m_Value)))
					{
						int* dataVal = m_Engine.TryGetPropertyFromSystem<int>(entity, property.second.m_ComponentType, property.second.m_PropertyName);
						if (dataVal == nullptr) continue;
						*dataVal = std::lerp(*currentVal, *nextVal, lerpVal);
					}
					else if (float* currentVal = std::get_if<float>(&(property.second.m_Value)); 
							 float* nextVal = std::get_if<float>(&(nextProperty->m_Value)))
					{
						float* dataVal = m_Engine.TryGetPropertyFromSystem<float>(entity, property.second.m_ComponentType, property.second.m_PropertyName);
						if (dataVal == nullptr) continue;
						*dataVal = std::lerp(*currentVal, *nextVal, lerpVal);
					}
					else if (std::uint8_t* currentVal = std::get_if<std::uint8_t>(&(property.second.m_Value));
						std::uint8_t* nextVal = std::get_if<std::uint8_t>(&(nextProperty->m_Value)))
					{
						std::uint8_t* dataVal = m_Engine.TryGetPropertyFromSystem<std::uint8_t>(entity, property.second.m_ComponentType, property.second.m_PropertyName);
						if (dataVal == nullptr)
						{
							Utils::Log(Utils::LogType::Warning, std::format("ANIMATOR LIGHT HAS NULL DATA VALUE FOUND FROM ENGINE"));
							continue;
						}
						*dataVal = std::lerp(*currentVal, *nextVal, lerpVal);
						Utils::Log(Utils::LogType::Warning, std::format("ANIMATOR HAS VALUE FOR LIGHT: {}", std::to_string(*dataVal)));

						//TODO: this should BE TEMPORARY AND NEEDS TO BE DONE ABSTRACTED PROBABLY USING ENGINE FUNCTION
						LightSourceData* data = entity.TryGetComponent<LightSourceData>();
						data->m_DataMutated = true;
					}
					else
					{
						Utils::Log(Utils::LogType::Error, std::format("Tried to update property in animator for entity:{} "
							"but either that property type is not defined or supported!", entity.m_Name));
					}
				}
			});	
	}

	bool AnimatorSystem::IsValidKeyframeIndex(const AnimatorData& data, const size_t& index) const
	{
		return 0 <= index && index < data.m_Keyframes.size();
	}

	//TODO: maybe optimize this by returning ref instead of pointer and having valid index as a precondition
	AnimationKeyframe& AnimatorSystem::GetNextKeyFrameAtIndex(AnimatorData& data, const size_t& index) const
	{
		size_t nextKeyframeIndex = data.m_KeyframeIndex + 1 % data.m_Keyframes.size();
		return data.m_Keyframes[nextKeyframeIndex];
	}

	std::optional<size_t> AnimatorSystem::TryGetKeyFrameAtTime(const AnimatorData& data, const float& time) const
	{
		if (data.m_Keyframes.empty()) return std::nullopt;
		if (data.m_Keyframes.size() == 1)
		{
			if (data.m_Keyframes.front().m_Time == time) return 0;
			else return std::nullopt;
		}
		float firstTime = data.m_Keyframes.front().m_Time;
		if (time < firstTime) return std::nullopt;
		if (Utils::ApproximateEqualsF(time, firstTime)) return 0;

		if (time > data.m_EndTime) return std::nullopt;
		if (Utils::ApproximateEqualsF(data.m_EndTime, firstTime)) return data.m_Keyframes.size() - 1;

		int left = 1;
		int right = data.m_Keyframes.size() - 1;
		int middle = 0;
		while (left <= right && left>=1 && right <data.m_Keyframes.size())
		{
			middle = (right - left) / 2 + left;
			if (Utils::ApproximateEqualsF(data.m_Keyframes[middle].m_Time, time))
				return middle;

			if (middle != 0 && data.m_Keyframes[middle - 1].m_Time <= time &&
				time <= data.m_Keyframes[middle].m_Time)
			{
				return middle - 1;
			}
			else if (data.m_Keyframes[middle].m_Time < time)
			{
				left = middle + 1;
			}
			else if (data.m_Keyframes[middle].m_Time > time)
			{
				right = middle - 1;
			}
		}
		return std::nullopt;
	}

	AnimationProperty* AnimatorSystem::TryFindMatchingProperty(AnimationKeyframe& keyframe, const AnimationProperty& property)
	{
		auto result= keyframe.m_Properties.find(property.m_PropertyName);
		if (result == keyframe.m_Properties.end()) return nullptr;
		else return &(result->second);
	}
}

