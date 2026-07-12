#include "Math/PlatformMath.hpp"
#include "Math/Math3d.hpp"

namespace Engine::Math::Platforms
{
	Mat4 CalculatePlatformPerspectiveProjMatrix(const float fovY, const float aspectRatio, const float zNear, const float zFar)
	{
#if defined(OPENGL)
		return CalculatePerspectiveProjMatrix(NdcRange::NegOneToOne, ZForwardSign::Negative, fovY, aspectRatio, zNear, zFar);
#elif defined(DIRECTX)
		return CalculatePerspectiveProjMatrix(NdcZRange::ZeroToOne, ZForwardSign::Positive, fovY, aspectRatio, zNear, zFar);
#elif defined(VULKAN)
		return CalculatePerspectiveProjMatrix(NdcZRange::ZeroToOne, ZForwardSign::Positive, fovY, aspectRatio, zNear, zFar);
#elif defined(METAL)
		return CalculatePerspectiveProjMatrix(NdcZRange::ZeroToOne, ZForwardSign::Positive, fovY, aspectRatio, zNear, zFar);
#else
#error "Graphics API does not have perspective view matrix defined"
		return {};
#endif
	}

	Mat4 CalculatePlatformOrthographicProjMatrix(const float maxWorldX, const float minWorldX, const float maxWorldY, 
		const float minWorldY, const float zNear, const float zFar)
	{
		//NOTE: when constructing matrix for this platform, we must convert to the RENDER API's NDC RANGE
		//but to do so we MUST PROVIDE THE ENGINE's Z DIRECTION
#if defined(OPENGL)
		return CalculateOrthographicProjMatrix(NdcRange::NegOneToOne, ZForwardSign::Negative, maxWorldX, minWorldX, maxWorldY, minWorldY, zNear, zFar);
#elif defined(DIRECTX)
		return CalculateOrthographicProjMatrix(NdcZRange::ZeroToOne, ZForwardSign::Positive, maxWorldX, minWorldX, maxWorldY, minWorldY, zNear, zFar);
#elif defined(VULKAN)
		return CalculateOrthographicProjMatrix(NdcZRange::ZeroToOne, ZForwardSign::Positive, maxWorldX, minWorldX, maxWorldY, minWorldY, zNear, zFar);
#elif defined(METAL)
		return CalculateOrthographicProjMatrix(NdcZRange::ZeroToOne, ZForwardSign::Positive, maxWorldX, minWorldX, maxWorldY, minWorldY, zNear, zFar);
#else
#error "Graphics API does not have perspective view matrix defined"
		return {};
#endif
	}
}