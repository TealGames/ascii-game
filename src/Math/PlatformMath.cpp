#include "Math/PlatformMath.hpp"

namespace PlatformMath
{
	Mat4 CalculatePerspectiveViewMatrix(const NdcRange ndcZRange, const ZForwardSign sign, 
		const float f, const float aspect, const float zNear, const float zFar)
	{
		std::array<std::array<float, 4>, 4> matrix =
		{ {
			{{f/aspect, 0,	0,	0}},
			{{0,		f,	0,	0}},
			{{0,		0,	0,	0}},
			{{0,		0,	0,	0}}
		} };

		//Right handedness
		if (sign == ZForwardSign::Negative)
		{
			if (ndcZRange == NdcRange::ZeroToOne)
			{
				matrix[2] = { {0, 0, zFar / (zNear - zFar), (zNear * zFar) / (zNear - zFar)} };
			}
			else
			{
				matrix[2] = { {0, 0, (zFar + zNear) / (zNear - zFar), (2* zFar * zNear) / (zNear - zFar)} };
			}
			matrix[3] = { {0, 0, -1, 0} };
		}
		//Left handedness
		else
		{
			if (ndcZRange == NdcRange::ZeroToOne)
			{
				matrix[2] = { {0, 0, zFar / (zFar - zNear), (-zNear * zFar) / (zFar- zNear)} };
			}
			else
			{
				matrix[2] = { {0, 0, (zNear + zFar) / (zFar - zNear), (-2* zNear * zFar) / (zFar - zNear)} };
			}
			matrix[3] = { {0, 0, 1, 0} };
		}
		return Mat4(matrix);
	}

	Mat4 CalculateOrthographicViewMatrix(const NdcRange ndcZRange, const ZForwardSign sign, 
		const float r, const float l, const float t, const float b, const float zNear, const float zFar)
	{
		std::array<std::array<float, 4>, 4> matrix =
		{ {
			{{2 / (r-l),	0,			0,		-(r+l)/(r-l)}},
			{{0,			2/(t-b),	0,		-(t+b)/(t-b)}},
			{{0,			0,			0,		0}},
			{{0,			0,			0,		1}}
		} };

		//Right handedness
		if (sign == ZForwardSign::Negative)
		{
			if (ndcZRange == NdcRange::ZeroToOne)
			{
				matrix[2] = { {0, 0, -1/ (zFar - zNear), -zNear/ (zFar - zNear)} };
			}
			else
			{
				matrix[2] = { {0, 0, -2 / (zFar - zNear), -(zFar + zNear)/(zFar - zNear)}};
			}
		}
		//Left handedness
		else
		{
			if (ndcZRange == NdcRange::ZeroToOne)
			{
				matrix[2] = { {0, 0, 1 / (zFar - zNear), -zNear / (zFar - zNear)} };
			}
			else
			{
				matrix[2] = { {0, 0, 2/(zFar- zNear), -(zFar + zNear) / (zFar - zNear)}};
			}
		}
		return Mat4(matrix);
	}

	Mat4 CalculatePlatformPerspectiveViewMatrix(const float fovY, const float aspectRatio, const float zNear, const float zFar)
	{
#if defined(OPENGL)
		return CalculatePerspectiveViewMatrix(NdcRange::NegOneToOne, ZForwardSign::Negative, fovY, aspectRatio, zNear, zFar);
#elif defined(DIRECTX)
		return CalculatePerspectiveViewMatrix(NdcZRange::ZeroToOne, ZForwardSign::Positive, fovY, aspectRatio, zNear, zFar);
#elif defined(VULKAN)
		return CalculatePerspectiveViewMatrix(NdcZRange::ZeroToOne, ZForwardSign::Negative, fovY, aspectRatio, zNear, zFar);
#elif defined(METAL)
		return CalculatePerspectiveViewMatrix(NdcZRange::ZeroToOne, ZForwardSign::Negative, fovY, aspectRatio, zNear, zFar);
#else
#error "Graphics API does not have perspective view matrix defined"
		return {};
#endif
	}

	Mat4 CalculatePlatformOrthographicViewMatrix(const float maxWorldX, const float minWorldX, const float maxWorldY, const float minWorldY, const float zNear, const float zFar)
	{
#if defined(OPENGL)
		return CalculateOrthographicViewMatrix(NdcRange::NegOneToOne, ZForwardSign::Negative, maxWorldX, minWorldX, maxWorldY, minWorldY, zNear, zFar);
#elif defined(DIRECTX)
		return CalculateOrthographicViewMatrix(NdcZRange::ZeroToOne, ZForwardSign::Positive, maxWorldX, minWorldX, maxWorldY, minWorldY, zNear, zFar);
#elif defined(VULKAN)
		return CalculateOrthographicViewMatrix(NdcZRange::ZeroToOne, ZForwardSign::Negative, maxWorldX, minWorldX, maxWorldY, minWorldY, zNear, zFar);
#elif defined(METAL)
		return CalculateOrthographicViewMatrix(NdcZRange::ZeroToOne, ZForwardSign::Negative, maxWorldX, minWorldX, maxWorldY, minWorldY, zNear, zFar);
#else
#error "Graphics API does not have perspective view matrix defined"
		return {};
#endif
	}
}