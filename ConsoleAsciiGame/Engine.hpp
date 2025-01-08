#pragma once
#include <chrono>
#include <optional>
#include <cstdint>
#include "SceneManager.hpp"
#include "TransformSystem.hpp"
#include "EntityRendererSystem.hpp"
#include "CameraSystem.hpp"
#include "LightSourceSystem.hpp"
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
	};
}


