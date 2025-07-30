#pragma once
#include <cstdint>
#include "Utils/Data/Matrix.hpp"
#include "Utils/Data/Vec3Type.hpp"

//Note: THE NDC AND Z FORWARD CAN NOT BE CHANGED BECAUSE PlatformMath FUNCTIONS 
// USE THIS TO CONVERT INTO RESPECTIVE RENDER FRAMEWORK PROJECTION MATRICES

/// <summary>
/// The range of normalized device coordinates
/// </summary>
enum class NdcRange : std::uint8_t
{
	ZeroToOne	= 0,
	NegOneToOne	= 1
};
/// <summary>
/// The normalized device coordinates for z axis for this engine.
/// It must be maintained across systems for rendering consistency (but is used as reference
/// point for conversion into different ndc ranges for different rendering platforms)
/// </summary>
inline constexpr NdcRange ENGINE_NDC_RANGE_Z = NdcRange::ZeroToOne;

//Note: the x and y ndc coords are the same for all major renderers so this is not as import as z
inline constexpr NdcRange ENGINE_NDC_RANGE_Y = NdcRange::NegOneToOne;
inline constexpr NdcRange ENGINE_NDC_RANGE_X = NdcRange::NegOneToOne;

/// <summary>
/// The sign of z as you move forward by default.
/// LEFT HANDEDNESS = positive
/// RIGHT HANDEDNESS = negative
/// </summary>
enum class ZForwardSign : std::uint8_t
{
	Positive	= 0,
	Negative	= 1,
};
inline constexpr ZForwardSign ENGINE_FORWARD_SIGN_Z = ZForwardSign::Negative;
inline constexpr Vec3 ENGINE_FORWARD_DIR = ENGINE_FORWARD_SIGN_Z == ZForwardSign::Negative ? Vec3(0, 0, -1) : Vec3(0, 0, 1);

namespace PlatformMath
{
	Mat4 CalculatePerspectiveViewMatrix(const NdcRange ndcZRange, const ZForwardSign sign, 
		const float fovY, const float aspectRatio, const float zNear, const float zFar);
	Mat4 CalculateOrthographicViewMatrix(const NdcRange ndcZRange, const ZForwardSign sign, 
		const float maxWorldX, const float minWorldX, const float maxWorldY, const float minWorldY, const float zNear, const float zFar);

	Mat4 CalculatePlatformPerspectiveViewMatrix(const float fovY, const float aspectRatio, const float zNear, const float zFar);
	Mat4 CalculatePlatformOrthographicViewMatrix(const float maxWorldX, const float minWorldX, const float maxWorldY, const float minWorldY, const float zNear, const float zFar);
}