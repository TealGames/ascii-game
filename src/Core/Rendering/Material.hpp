#pragma once
#include "Utils/Data/Color.hpp"

namespace Rendering
{
	struct Material
	{
		/// <summary>
		/// The base color for the object (also known as diffuse base color)
		/// If this is empty, default 1x1 pixel white texture is selected
		/// </summary>
		Texture* m_Albedo = nullptr;

		/// <summary>
		/// The color overlay over the albedo.
		/// </summary>
		Utils::Color m_BaseColor;

		/// <summary>
		/// Value between [0, 1] describing roughness/smoothness
		/// on a microscopic level including reflection glossiness/spread where
		/// 0-> super smooth/shiny with sharp reflections (ex. mirror)
		/// 1-> super rough/matte with blurry reflections (chalk)
		/// </summary>
		float m_Roughness = 0.0f;

		/// <summary>
		/// Value between [0, 1] describing how metal-like (or dielectric/non-metal)
		/// the object is where 
		/// 0-> non-metal with white specular reflections, have albedo base color (ex. wood)
		/// 1-> metal with colored specular relfections, reflection color is albedo, no diffuse (ex. gold)
		/// 
		/// *Specular Light: reflections of light sources on object
		/// *Diffuse light: light hitting rough surface and scattering evenly in all directions
		/// </summary>
		float m_Metallic = 0.0f;

		/// <summary>
		/// a texture describing the surface of the object
		/// which is used for lighting
		/// </summary>
		Texture* m_NormalMap = nullptr;
	};
}