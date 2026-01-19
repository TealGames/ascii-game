#pragma once
#include <cstdint>
#include "Utils/Math/Matrix.hpp"
#include "Utils/Math/Vec3Type.hpp"

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
/// The normalized device coordinates for all axes in XYZ order for this engine.
/// It must be maintained across systems for rendering consistency (but is used as reference
/// point for conversion into different ndc ranges for different rendering platforms)
/// </summary>
inline constexpr std::array<NdcRange, 3> ENGINE_NDC_RANGES = { NdcRange::NegOneToOne, NdcRange::NegOneToOne, NdcRange::ZeroToOne };

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
inline constexpr ZForwardSign ENGINE_FORWARD_SIGN_Z = ZForwardSign::Positive;
inline constexpr Vec3 ENGINE_FORWARD_DIR = ENGINE_FORWARD_SIGN_Z == ZForwardSign::Negative ? Vec3(0, 0, -1) : Vec3(0, 0, 1);
inline constexpr Vec3 ENGINE_UP_DIR = Vec3(0, 1, 0);
inline constexpr Vec3 ENGINE_RIGHT_DIR = Vec3(1, 0, 0);
inline constexpr Vec3 ENGINE_RIGHT_UP_FORWARD_DIR = ENGINE_RIGHT_DIR + ENGINE_UP_DIR + ENGINE_FORWARD_DIR;

namespace PlatformMath
{
	Mat4 CalculatePerspectiveProjMatrix(const NdcRange ndcZRange, const ZForwardSign sign, 
		const float fovY, const float aspectRatio, const float zNear, const float zFar);
	Mat4 CalculateOrthographicProjMatrix(const NdcRange ndcZRange, const ZForwardSign sign, 
		const float maxWorldX, const float minWorldX, const float maxWorldY, const float minWorldY, const float zNear, const float zFar);

	Mat4 CalculatePlatformPerspectiveProjMatrix(const float fovY, const float aspectRatio, const float zNear, const float zFar);
	Mat4 CalculatePlatformOrthographicProjMatrix(const float maxWorldX, const float minWorldX, const float maxWorldY, const float minWorldY, const float zNear, const float zFar);
}