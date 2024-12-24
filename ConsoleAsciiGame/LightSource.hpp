#pragma once
#include <vector>
#include <cstdint>
#include <limits>
#include "raylib.h"
#include "Component.hpp"
#include "Renderer.hpp"
#include "Transform.hpp"
#include "LightSource.hpp"
#include "Point2DInt.hpp"
#include "TextBuffer.hpp"

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

	class LightSource : public Component
	{
	private:
		//std::vector<PositionLightData> m_lightData;
		const Renderer& m_renderer;
		const Transform& m_transform;
		std::vector<TextBuffer*> m_outputBuffers;
		const int m_lightRadius;
		const Color m_filterColor;

		//The strength of the light initially
		std::uint8_t m_intensity;

		//where <1 creates more logarithmic curves, 
		//=1 creates linear and >1 creates exponential decay
		//and -1 means unused
		float m_falloffStrength;

	public:


	private:
		void RenderLight(bool displayLightLevels=false) const;
		std::uint8_t CalculateLightLevelFromDistance(const float& distance) const;
		Color CalculateNewColor(const TextBuffer& buffer, const Utils::Point2DInt& currentPos,
			const Utils::Point2DInt& centerPos, std::uint8_t* outLightLevel=nullptr) const;

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
		LightSource(const Transform& transform, const Renderer& renderer, 
			const std::vector<TextBuffer*>& outputBuffers, const Color& filterColor, const int& lightRadius,
			const std::uint8_t& initialLightLevel, const float& falloffValue);

		UpdatePriority GetUpdatePriority() const override;

		void Start() override;
		void Update(float deltaTime) override;
	};

}

