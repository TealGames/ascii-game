#pragma once
#include <vector>
#include <cstdint>
#include <optional>
#include <limits>
#include "raylib.h"
#include "Component.hpp"
#include "MultiBodySystem.hpp"
#include "EntityRendererSystem.hpp"
#include "TransformSystem.hpp"
#include "LightSourceSystem.hpp"
#include "Point2DInt.hpp"
#include "Point4D.hpp"
#include "TextBuffer.hpp"
#include "ColorGradient.hpp"
#include "LightSourceData.hpp"
#include "Scene.hpp"

namespace ECS
{
	constexpr std::uint8_t MIN_LIGHT_LEVEL = std::numeric_limits< std::uint8_t>::min();
	constexpr std::uint8_t MAX_LIGHT_LEVEL = std::numeric_limits< std::uint8_t>::max();

	/*struct PositionLightData
	{
		Utils::Point2DInt LocalPos;
		uint8_t LightLevel;
	};*/

	enum class LightShape
	{
		Circle,
	};

	class LightSourceSystem : MultiBodySystem
	{
	private:
		TransformSystem m_transformSystem;
		EntityRendererSystem m_rendererSystem;

	public:


	private:
		/// <summary>
		/// This will calculate a new color based on the original color and filter color and the strength of the color
		/// </summary>
		/// <param name="originalColor"></param>
		/// <param name="filterColor"></param>
		/// <param name="multiplier"></param>
		/// <returns></returns>
		Color GetColorFromMultiplier(const Color& originalColor, const Color& filterColor, const float& multiplier) const;

		/// <summary>
		/// This will calculate a new color by combining the fractional color with the original color
		/// </summary>
		/// <param name="originalColor"></param>
		/// <param name="filterColor"></param>
		/// <returns></returns>
		Color ApplyColorFilter(const Color& originalColor, const Utils::Point3D& fractionalColor, const float& multiplier) const;

		void CreateLightingForPoint(LightSourceData& data, const ECS::Entity& entity, const Utils::Point2DInt& centerPos,
			TextBuffer& buffer, bool displayLightLevels);

		void RenderLight(LightSourceData& data, ECS::Entity& entity, std::vector<TextBuffer*>& buffers, bool displayLightLevels = false);
		std::uint8_t CalculateLightLevelFromDistance(const LightSourceData& data, const float& distance) const;
		Color CalculateNewColor(LightSourceData& data, const ECS::Entity& entity, const TextBuffer& buffer, const Utils::Point2DInt& currentPos,
			const Utils::Point2DInt& centerPos, std::uint8_t* outLightLevel = nullptr, LightMapChar* lightMapChar=nullptr) const;

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
		LightSourceSystem(const TransformSystem& transform, const EntityRendererSystem& renderer);

		void SystemUpdate(Scene& scene, const float& deltaTime) override;
	};

}

