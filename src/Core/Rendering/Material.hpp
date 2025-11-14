#pragma once
#include "Utils/Data/Color.hpp"
#include "Utils/Data/FixedString.hpp"

namespace Rendering
{
	class Texture;
	struct Material
	{
	private:
		/// <summary>
		/// The color overlay over the albedo.
		/// NOTE: base color alpha only determines the intensity/blend
		/// of this color over the albedo
		/// </summary>
		Color m_baseColor;

		/// <summary>
		/// The intensity and color of light that the surface emits.
		/// This makes any object able to emit lights rather than defining them separately
		/// </summary>
		Color m_emissiveColor;

		/// <summary>
		/// The alpha applied to the whole object
		/// </summary>
		float m_alpha;

		/// <summary>
		/// Value between [0, 1] describing roughness/smoothness
		/// on a microscopic level including reflection glossiness/spread where
		/// 0-> super smooth/shiny with sharp reflections (ex. mirror)
		/// 1-> super rough/matte with blurry reflections (chalk)
		/// </summary>
		float m_roughness = 0.0f;
		
		/// <summary>
		/// Value between [0, 1] describing how metal-like (or dielectric/non-metal)
		/// the object is where 
		/// 0-> non-metal with white specular reflections, have albedo base color (ex. wood)
		/// 1-> metal with colored specular relfections, reflection color is albedo, no diffuse (ex. gold)
		/// NOTE: to be as physically accurate, you should use ONLY 0 or 1 (because object behaving half metal does not make sense)
		/// 
		/// *Specular Light: reflections of light sources on object
		/// *Diffuse light: light hitting rough surface and scattering evenly in all directions
		/// </summary>
		float m_metallic = 0.0f;
	public:
		//NOTE: we do not allow roughness of 0.01 because it causes division by 0 in math equations
		static constexpr float MIN_ROUGHNESS = 0.01f;
		static constexpr float MAX_ROUGHNESS = 1.0f;
		static constexpr float MIN_METALLIC = 0.0f;
		static constexpr float MAX_METALLIC = 1.0f;

		static constexpr Color DEFAULT_BASE_COLOR = COLOR_WHITE;
		static constexpr Color DEFAULT_EMISSIVE_COLOR = COLOR_BLACK;
		static constexpr float DEFAULT_ALPHA = 1;
		static constexpr float DEFAULT_ROUGHNESS = MAX_ROUGHNESS;
		static constexpr float DEFAULT_METALLIC = MIN_METALLIC;

		String16 m_Name;

		/// <summary>
		/// The base color for the object (also known as diffuse base color)
		/// If this is empty, default 1x1 pixel white texture is selected
		/// </summary>
		Texture* m_Albedo;

		/// <summary>
		/// a texture describing the surface of the object
		/// which is used for lighting
		/// </summary>
		Texture* m_NormalMap;
	private:
	public:
		Material();
		Material(const String16& name, Texture* albedo, const Color& baseColor, const float alpha = DEFAULT_ALPHA, 
			const Color& emissiveColor = DEFAULT_EMISSIVE_COLOR, const float roughness = DEFAULT_ROUGHNESS, 
			const float metallic = DEFAULT_METALLIC, Texture* normal = nullptr);

		void SetAlpha(const float value);
		float GetAlpha() const;

		void SetBaseColor(const Color& color);
		const Color& GetBaseColor() const;

		void SetEmissiveColor(const Color& color);
		const Color& GetEmissiveColor() const;

		void SetRoughness(const float value);
		float GetRoughness() const;

		void SetMetallic(const float value);
		float GetMatallic() const;

		void SetSurface(const float roughness, const float metallic, Texture* normalMap);

		std::string ToString() const;
	};
}