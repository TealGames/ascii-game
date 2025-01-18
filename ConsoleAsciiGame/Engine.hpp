#pragma once
#include <chrono>
#include <optional>
#include <format>
#include <string>
#include <cstdint>
#include <type_traits>
#include "SceneManager.hpp"
#include "TransformSystem.hpp"
#include "EntityRendererSystem.hpp"
#include "CameraSystem.hpp"
#include "LightSourceSystem.hpp"
#include "AnimatorSystem.hpp"
#include "SpriteAnimatorSystem.hpp"
#include "PlayerSystem.hpp"

namespace Core
{
	using LoopCode = std::uint8_t;
	class Engine
	{
	private:

		SceneManagement::SceneManager m_sceneManager;

		std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_currentTime;
		double m_deltaTime = 0;
		int m_currentFPS = 0;

		//TODO: there has to be a way that does not involve us writing every possible system
		ECS::TransformSystem m_transformSystem;
		ECS::CameraSystem m_cameraSystem;
		ECS::LightSourceSystem m_lightSystem;
		ECS::PlayerSystem m_playerSystem;
		ECS::EntityRendererSystem m_entityRendererSystem;
		ECS::AnimatorSystem m_animatorSystem;
		ECS::SpriteAnimatorSystem m_spriteAnimatorSystem;

		std::optional<ECS::EntityComponentPair<PlayerData>> m_playerInfo;
		std::optional<ECS::EntityComponentPair<CameraData>> m_mainCameraInfo;

		std::uint8_t m_currentFrameCounter;

	public:

	private:
		void Init();
		void Destroy();

		/// <summary>
		/// Will run one update loop.
		/// Returns true if there was an error, or the loop prevented further execution
		/// Returns false if it can continue
		/// </summary>
		/// <returns></returns>
		LoopCode Update();

	public:
		Engine();
		~Engine();

		void BeginUpdateLoop();

		//TODO: this should probably get abstracted out into a reflection manager that stores type sand their properties
		//and all systems would submit their properties and types/other metadata to it on init of engine
		template<typename PropertyType>
		PropertyType* TryGetPropertyFromSystem(ECS::Entity& entity, const ComponentType& type, const std::string& propertyName)
		{
			if (type == ComponentType::LightSource)
			{
				LightSourceData* maybeData = entity.TryGetComponent<LightSourceData>();
				if (!Utils::Assert(this, maybeData != nullptr, std::format("Tried to get property: {} from system for entity: {} and component: {} "
					"but it does not have that component", propertyName, entity.m_Name, ToString(type)))) return nullptr;

				if (propertyName == "LightRadius" && std::is_same_v<PropertyType, decltype(maybeData->m_LightRadius)>)
					//reinterpret cast is dangerous but we have to do it here since we know for sure if the condition of the same
					//type sxecutes we can be sure we can convert to this type since we cant just return normally since compiler is not sure whether types match
					return reinterpret_cast<PropertyType*>(&(maybeData->m_LightRadius));
				if (propertyName == "LightIntensity" && std::is_same_v<PropertyType, decltype(maybeData->m_Intensity)>)
					return reinterpret_cast<PropertyType*>(&(maybeData->m_Intensity));
				else
				{
					if (!Utils::Assert(this, maybeData != nullptr, std::format("Tried to get property: {} from system for entity: {} and component: {} "
						"but it did not match any names and/or their types with type: {}!", propertyName, 
						entity.m_Name, ToString(type), typeid(PropertyType).name()))) return nullptr;
				}
			}
			else
			{
				Utils::Log(Utils::LogType::Error, std::format("Tried to get property: {} from "
					"engine for entity: {} of an undefined type: {}", propertyName, entity.m_Name, ToString(type)));
			}
			return nullptr;
		}
	};
}


