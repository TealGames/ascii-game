#pragma once
#include <vector>
#include <cstdint>
#include <optional>
#include <limits>
//#include "raylib.h"
#include "ECS/Component/Component.hpp"
#include "ECS/Systems/Types/World/EntityRenderer2DSystem.hpp"
#include "ECS/Systems/Types/World/TransformSystem.hpp"
#include "ECS/Systems/Types/World/LightSource2DSystem.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Core/Primitives/ColorGradient.hpp"
#include "ECS/Component/Types/World/LightSource2DComponent.hpp"
#include "Core/Scene/Scene.hpp"

namespace Engine::ECS { class EntityData; }
namespace Engine::Lighting2D
{
	constexpr std::uint8_t MIN_LIGHT_LEVEL = std::numeric_limits< std::uint8_t>::min();
	constexpr std::uint8_t MAX_LIGHT_LEVEL = std::numeric_limits< std::uint8_t>::max();

	enum class LightShape : std::uint8_t
	{
		Circle,
	};

	class LightSource2DSystem
	{
	private:
		const Rendering::EntityRenderer2DSystem& m_rendererSystem;
	public:

	private:
		/// <summary>
		/// This will calculate a new color based on the original color and filter color and the strength of the color
		/// </summary>
		/// <param name="originalColor"></param>
		/// <param name="filterColor"></param>
		/// <param name="multiplier"></param>
		/// <returns></returns>
		ColHDR4 GetColorFromMultiplier(const ColHDR4& originalColor, const ColHDR4& filterColor, const float& multiplier) const;

		void CreateLightingForPoint(LightSource2DComponent& data, const WorldPosition3D& centerPos,
			Rendering::FragmentedTextBuffer2D& buffer, bool displayLightLevels);

		void RenderLight(LightSource2DComponent& data, std::vector<Rendering::FragmentedTextBuffer2D*>& buffers, bool displayLightLevels = false);
		std::uint8_t CalculateLightLevelFromDistance(const LightSource2DComponent& data, const float& distance) const;
		ColHDR4 CalculateNewColor(LightSource2DComponent& data, const Rendering::TextBufferCharPosition2D& bufferPos, const float& distance,
			std::uint8_t* outLightLevel = nullptr, LightMapChar* lightMapChar=nullptr) const;

	public:
		/// <summary>
		/// Creates a light source
		/// </summary>
		/// <param name="transform"></param>
		/// <param name="renderer"></param>
		/// <param name="outputBuffers">The layers/buffers that will have this glow applied to</param>
		/// <param name="filterColor">The tint of light that is applied in the sorrounding area</param>
		/// <param name="lightRadius">The max reach of any possible light for this source</param>
		/// <param name="initialLightLevel">The light level that is present at the object's center/transform position</param>
		/// <param name="falloffValue">THe rate that the light will fade away where <1 creates more logarithmic curves, 
		//=1 creates linear and >1 creates exponential decay</param>
		LightSource2DSystem(const Rendering::EntityRenderer2DSystem& renderer);

		void SystemUpdate(Scenes::Scene& scene, Camera::CameraComponent& mainCamera, const float& deltaTime);
	};

}

