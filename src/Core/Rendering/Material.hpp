#pragma once
#include "Core/Asset/TextureAsset.hpp"

namespace Rendering
{
	struct Material
	{
		/// <summary>
		/// The base color for the object (also known as diffuse base color)
		/// </summary>
		TextureAsset* m_Albedo = nullptr;
		/// <summary>
		/// Value between [0, 1] describing roughness/smoothness
		/// on a microscopic level including reflection glossiness/spread where
		/// 0-> super smooth/shiny with sharp reflections (ex. mirror)
		/// 1-> super rough/matte with blurry reflections (chalk)
		/// </summary>
		float m_Roughness = 0f;

		/// <summary>
		/// Value between [0, 1] describing how metal-like (or dielectric/non-metal)
		/// the object is where 
		/// 0-> non-metal with white specular reflections, have albedo base color (ex. wood)
		/// 1-> metal with colored specular relfections, reflection color is albedo, no diffuse (ex. gold)
		/// 
		/// *Specular Light: reflections of light sources on object
		/// *Diffuse light: light hitting rough surface and scattering evenly in all directions
		/// </summary>
		float m_Metallic = 0f;

		/// <summary>
		/// a texture describing the surface of the object
		/// which is used for lighting
		/// </summary>
		TextureAsset* m_NormalMap = nullptr;
	};
}