#include "Math/Math3d.hpp"
#include "Utils/Math.hpp"

namespace Engine::Math
{
	Mat4 CalculatePerspectiveProjMatrix(const NdcRange ndcZRange, const ZForwardSign sign,
		const float f, const float aspect, const float zNear, const float zFar)
	{
		const float fov_scale = 1 / std::tan(f / 2);
		std::array<std::array<float, 4>, 4> matrix =
		{ {
			{{fov_scale / aspect, 0,			0,	0}},
			{{0,				 fov_scale,	0,	0}},
			{{0,				 0,			0,	0}},
			{{0,				 0,			0,	0}}
		} };

		//Right handedness
		if (sign == ZForwardSign::Negative)
		{
			if (ndcZRange == NdcRange::ZeroToOne)
			{
				matrix[2] = { {0, 0, -zFar / (zNear - zFar), -zNear * zFar / (zNear - zFar)} };
				matrix[3] = { {0, 0, -1, 0} };
			}
			//-1 to 1
			else
			{
				matrix[2] = { {0, 0, (zFar + zNear) / (zNear - zFar), (2 * zFar * zNear) / (zNear - zFar)} };
				matrix[3] = { {0, 0, -1, 0} };
			}
		}
		//Left handedness
		else
		{
			if (ndcZRange == NdcRange::ZeroToOne)
			{
				matrix[2] = { {0, 0, zFar / (zFar - zNear), (-zNear * zFar) / (zFar - zNear)} };
				matrix[3] = { {0, 0, 1, 0} };
			}
			//-1 to 1
			else
			{
				matrix[2] = { {0, 0, (zNear + zFar) / (zFar - zNear), (-2 * zNear * zFar) / (zFar - zNear)} };
				matrix[3] = { {0, 0, 1, 0} };
			}
		}
		/*LogError(std::format("Matches:{}", Mat4(matrix).ToString()));*/
		return Mat4(matrix);
	}

	Mat4 CalculateOrthographicProjMatrix(const NdcRange ndcZRange, const ZForwardSign sign,
		const float r, const float l, const float t, const float b, const float zNear, const float zFar)
	{
		std::array<std::array<float, 4>, 4> matrix =
		{ {
			{{2 / (r - l),	0,			0,		-(r + l) / (r - l)}},
			{{0,			2 / (t - b),	0,		-(t + b) / (t - b)}},
			{{0,			0,			0,		0}},
			{{0,			0,			0,		1}}
		} };

		//Right handedness
		if (sign == ZForwardSign::Negative)
		{
			if (ndcZRange == NdcRange::ZeroToOne)
			{
				matrix[2] = { {0, 0, -1 / (zFar - zNear), zNear / (zFar - zNear)} };
			}
			else
			{
				matrix[2] = { {0, 0, -2 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear)} };
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
				matrix[2] = { {0, 0, 2 / (zFar - zNear), -(zFar + zNear) / (zFar - zNear)} };
			}
		}
		return Mat4(matrix);
	}

	static bool RayIntersectsTriangleBase(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Ray3D& ray, float* outTEnter,
		float* outU, float* outV, float* outW)
	{
		ENGINE_ASSERT(Math::IsUnitVector(ray.m_Dir), 
			"Attempted to invoke RayIntersectsTriangle but ray dir is not unit vector:{}", ray.m_Dir.ToString());

		const Vec3 e1 = v1 - v0;
		const Vec3 e2 = v2 - v0;
		const Vec3 p = CrossProduct(ray.m_Dir, e2);
		const float determinant = DotProduct(e1, p);
		if (fabs(determinant) < ::Math::EPSILON)
			return false;

		const float inverseDeterminant = 1.0f / determinant;
		const Vec3 tvec = ray.m_Origin - v0;
		const float u = DotProduct(tvec, p) * inverseDeterminant;
		if (u < 0 || u > 1)
			return false;

		const Vec3 q = CrossProduct(tvec, e1);
		const float v = DotProduct(ray.m_Dir, q) * inverseDeterminant;
		if (v < 0 || u + v > 1)
			return false;

		const float t = DotProduct(e2, q) * inverseDeterminant;
		if (t < 0)
			return false;

		if (outTEnter != nullptr)
			*outTEnter = t;

		if (outU != nullptr && outV != nullptr && outW != nullptr)
		{
			*outU = u;
			*outV = v;
			*outW = 1.0 - u - v;
		}

		return true;
	}
	bool RayIntersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Ray3D& ray, float* outTEnter)
	{
		return RayIntersectsTriangleBase(v0, v1, v2, ray, outTEnter, nullptr, nullptr, nullptr);
	}
	bool RayIntersectsTriangleInterpolated(const Vec3& v0, const Vec3& v1, const Vec3& v2, 
		const Vec3& n0, const Vec3 n1, const Vec3 n2, const Ray3D& ray, float* outTEnter, Vec3& outTriangleNormal)
	{
		float u, v, w;
		if (!RayIntersectsTriangleBase(v0, v1, v2, ray, outTEnter, &u, &v, &w))
			return false;

		outTriangleNormal = (n0 * w + n1 * u + n2 * v);
		return true;
	}
	bool RayIntersectsSphere(const Vec3& sphereCenter, const float radius, const Ray3D& ray, float* outTEnter)
	{
		ENGINE_ASSERT(Math::IsUnitVector(ray.m_Dir), 
			"Attempted to invoke RayIntersectsSphere but ray dir is not unit vector:{}", ray.m_Dir.ToString());

		//NOTE: the ray dir does not have to be normalized because it is just 
		//used in the equation of a 3d line <x,y,z> = Vo + Vt where V is the non-unit vector direction
		const Vec3 l = ray.m_Origin - sphereCenter;
		const float tca = DotProduct(l, ray.m_Dir);
		const float d2 = DotProduct(l, l) - tca * tca;
		if (d2 > radius * radius) 
			return false; 

		const float thc = sqrt(radius * radius - d2);
		const float t0 = -tca - thc;
		if (t0 > 0)
		{
			if (outTEnter != nullptr) *outTEnter = t0;
			return true;
		}
		const float t1 = -tca + thc;
		if (t1 > 0)
		{
			if (outTEnter != nullptr) *outTEnter = t1;
			return true;
		}
		return false;
	}

	float CalculateSurfaceArea(const Vec3& minPos, const Vec3& maxPos)
	{
		const float xDiff = maxPos.m_X - minPos.m_X;
		const float yDiff = maxPos.m_Y - minPos.m_Y;
		const float zDiff = maxPos.m_Z - minPos.m_Z;
		return 2 * (xDiff * yDiff + yDiff * zDiff + xDiff * zDiff);
	}

	float CalculateParallelogramArea(const Vec3& v0, const Vec3& v1, const Vec3& v2)
	{
		return CrossProduct(v1 - v0, v2 - v0).GetMagnitude();
	}
	float CalculateTriangleArea(const Vec3& v0, const Vec3& v1, const Vec3& v2)
	{
		return 0.5f * CalculateParallelogramArea(v0, v1, v2);
	}
	Vec3 CalculateBarycentricWeight(const Vec3& targetPos, const Vec3& v0, const Vec3& v1, const Vec3& v2)
	{
		/*
		 * Since 0.5 * cross product(v0, v1) is the area of a triangle
		 * we can use that to determine how big the area of the triangle formed between the targetPos
		 * and two adjacent vertices in relation to the FULL AREA to get ratios of how close
		 * a point is (closer to vertices -> smaller triangle area -> dividing smaller value -> greater fraction)
		 */

		//(NOTE: we use parallogram area (just doing cross(v0, v1, v2)) because we use ratios of small area over full area
		//the 0.5 would cancel out
		float area = CalculateParallelogramArea(v0, v1, v2);
		float a0 = CrossProduct(v1 - targetPos, v2 - targetPos).GetMagnitude() / area;
		float a1 = CrossProduct(v2 - targetPos, v0 - targetPos).GetMagnitude() / area;
		//NOTE: we skip the last area because we know all 3 values must equal 1
		float a2 = 1.0 - a0 - a1;
		return Vec3(a0, a1, a2);
	}


	Mat4 CalculateTranslationMatrix(const Vec3& pos)
	{
		return Mat4(std::array<std::array<float, 4>, 4>
		{{
			{ {1, 0, 0, pos.m_X} },
			{ {0, 1, 0, pos.m_Y} },
			{ {0, 0, 1, pos.m_Z} },
			{ {0, 0, 0, 1} }
		}});
	}

	Mat4 CalculateScaleMatrix(const Vec3& scale)
	{
		return Mat4(std::array<std::array<float, 4>, 4>
		{{
			{ {scale.m_X, 0, 0, 0} },
			{ {0, scale.m_Y, 0, 0} },
			{ {0, 0, scale.m_Z, 0} },
			{ {0, 0, 0, 1} }
		}});
	}

	Mat4 CalculateRotationMatrix(const Quat& rotation)
	{
		const float x = rotation.m_X, y = rotation.m_Y, z = rotation.m_Z, w = rotation.m_W;

		const float xx = x * x, yy = y * y, zz = z * z;
		const float xy = x * y, xz = x * z, yz = y * z;
		const float wx = w * x, wy = w * y, wz = w * z;

		return Mat4(std::array<std::array<float, 4>, 4>
			{ {
				{{1 - 2 * yy - 2 * zz,	2 * xy - 2 * wz,		2 * xz + 2 * wy,		0.0f}},
				{{2 * xy + 2 * wz,		1 - 2 * xx - 2 * zz,	2 * yz - 2 * wx,		0.0f}},
				{{2 * xz - 2 * wy,		2 * yz + 2 * wx,		1 - 2 * xx - 2 * yy,	0.0f}},
				{{0.0f,					0.0f,					0.0f,					1.0f}}
			} });
	}
	Mat4 CalculateRotationMatrix(const Vec3& forwardDir, const Vec3& upDir, const Vec3& rightDir)
	{
		Mat4 result = Mat4::GetIdentity();
		result.SetCol(0, rightDir);
		result.SetCol(1, upDir);
		result.SetCol(2, -forwardDir);
		return result;
	}

	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Quat& rotation)
	{
		ENGINE_ASSERT(scale.IsUniform(), "Non uniform scale in model matrices is not allowed due to raytracer ray distortion");
		return CalculateModelMatrix(parentMatrix, CalculateTranslationMatrix(pos), 
			CalculateScaleMatrix(scale), CalculateRotationMatrix(rotation));
	}
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Mat4& rotation)
	{
		ENGINE_ASSERT(scale.IsUniform(), "Non uniform scale in model matrices is not allowed due to raytracer ray distortion");
		return CalculateModelMatrix(parentMatrix, CalculateTranslationMatrix(pos),
			CalculateScaleMatrix(scale), rotation);
	}
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Mat4& posMatrix, const Mat4& scaleMatrix, const Mat4& rotationMatrix)
	{
		ENGINE_ASSERT(ExtractScaleFromMatrix(scaleMatrix).IsUniform(), "Non uniform scale in model matrices is not allowed due to raytracer ray distortion");
		if (parentMatrix == nullptr)
			return posMatrix * scaleMatrix * rotationMatrix;
		else
			return *parentMatrix * posMatrix * scaleMatrix * rotationMatrix;
	}
	Mat4 CalculateInverseModelMatrix(const Mat4& matrix)
	{
		const Mat3 inverseScaleRotation = matrix.GetSlice<3, 3>().InverseUnsafe();
		const Vec3 inverseTranslation = -inverseScaleRotation * ExtractTranslationFromMatrix(matrix);
		Mat4 inversedMatrix = Mat4::GetIdentity();
		inversedMatrix.SetTopLeft(inverseScaleRotation);
		inversedMatrix.SetCol(3, inverseTranslation);
		return inversedMatrix;
	}

	Mat3 CalculateTranslationMatrix(const Vec2& pos)
	{
		return Mat3(std::array<std::array<float, 3>, 3>
		{{
			{ {1, 0, pos.m_X} },
			{ {0, 1, pos.m_Y} },
			{ {0, 0, 1} }
		}});
	}
	Mat3 CalculateScaleMatrix(const Vec2& scale)
	{
		return Mat3(std::array<std::array<float, 3>, 3>
		{{
			{ {scale.m_X, 0, 0} },
			{ {0, scale.m_Y, 0} },
			{ {0, 0, 1} }
		}});
	}
	Mat3 CalculateUIModelMatrix(const Mat3* parentMatrix, const Vec2& bottomLeftPos, const Vec2& size, const Vec2& pivot)
	{
		return CalculateUIModelMatrix(parentMatrix, CalculateTranslationMatrix(-pivot), CalculateScaleMatrix(size),
			CalculateTranslationMatrix(pivot * size), CalculateTranslationMatrix(bottomLeftPos));
	}
	Mat3 CalculateUIModelMatrix(const Mat3* parentMatrix, const Mat3& pivotToOriginMoveMatrix, const Mat3& scaleMatrix,
		const Mat3& scaledOriginToPivotMoveMatrix, const Mat3& posMatrix)
	{
		//We first apply a translation to move the pivot to origin -> we scale the rect pos at pivot origin ->
		//we move pivot from origin back to original place in SCALED SPACE -> we apply translation
		if (parentMatrix != nullptr)
			return *parentMatrix * posMatrix * scaledOriginToPivotMoveMatrix * scaleMatrix * pivotToOriginMoveMatrix;
		return posMatrix * scaledOriginToPivotMoveMatrix * scaleMatrix * pivotToOriginMoveMatrix;
	}

	float CalculateTransformMatrixDeterminant(const Mat4& matrix)
	{
		return CalculateTransformMatrixDeterminant(matrix.GetColVector(0).GetXYZ(), 
			matrix.GetColVector(1).GetXYZ(), matrix.GetColVector(2).GetXYZ());
	}
	float CalculateTransformMatrixDeterminant(const Vec3& matrixCol0, const Vec3& matrixCol1, const Vec3& matrixCol2)
	{
		//If we have the columns, we can just do the 3x3 determinant which will be the same as the dot product
		// of the cross product
		return DotProduct(matrixCol0, CrossProduct(matrixCol1, matrixCol2));
	}

	Vec3 ExtractTranslationFromMatrix(const Mat4& matrix)
	{
		return matrix.GetColVector(3).GetXYZ();
	}
	Quat ExtractRotationFromMatrix(const Mat4& matrix, Vec3* outScale)
	{
		//NOTE: this function ONLY works if there is no shear since then we need to use other methods
		//to get the rotation part because with shear the axes are NOT orthonormal (are not perpendicular unit vectors)

		Vec3 col0 = matrix.GetColVector(0).GetXYZ();
		Vec3 col1 = matrix.GetColVector(1).GetXYZ();
		Vec3 col2 = matrix.GetColVector(2).GetXYZ();

		const float scaleX = col0.GetMagnitude();
		const float scaleY = col1.GetMagnitude();
		const float scaleZ = col2.GetMagnitude();
		if (outScale != nullptr) *outScale = Vec3(scaleX, scaleY, scaleZ);

		//We get rid of the scale's magnitude from the matrix with normalization
		//which accounts for both uniform/non-uniform scaling. However, negative scaling
		//can still affect the matrix, which is checked below with determinant
		col0 /= scaleX;
		col1 /= scaleY;
		col2 /= scaleZ;
		
		Mat3 rotationMat = {};
		rotationMat.SetCol(0, col0);
		rotationMat.SetCol(1, col1);
		rotationMat.SetCol(2, col2);
		const float determinant = CalculateTransformMatrixDeterminant(col0, col1, col2);
		//If the determinant is less than 1, it means we have some negative scaling 
		//and thus we must reverse one column to get rid of it 
		//NOTE: one switch is sufficient because negative determinant means matrix has opposite 
		//handedness, so by switching one column it flips the orientation back
		if (determinant < 0.0f)
		{
			rotationMat.SetCol(0, rotationMat.GetColVector(0) * -1);
			if (outScale != nullptr) outScale->m_X *= -1;
		}
		return ToQuaternion(rotationMat);
	}
	Vec3 ExtractScaleFromMatrix(const Mat4& matrix)
	{
		return Vec3{ matrix.GetColVector(0).GetXYZ().GetMagnitude(),
					 matrix.GetColVector(1).GetXYZ().GetMagnitude(),
					 matrix.GetColVector(2).GetXYZ().GetMagnitude() };
	}

	void ExtractTransformFromMatrix(const Mat4& matrix, Vec3& outPosition, Quat& outRotation, Vec3& outScale)
	{
		outPosition = ExtractTranslationFromMatrix(matrix);
		outRotation = ExtractRotationFromMatrix(matrix, &outScale);
	}

	Vec2 OctahedralEncodeNormal(const Vec3& normal)
	{
		//|x| + |y| + |z| = 1 is the equation of a octahedron (similar to Plumbob from Sims -> two pyramids glued at bases)
		//so we project the ray from sphere to octahedron surface -> same direction but different endpoint
		//by dividing (because octahedron is smaller within the bounds of a sphere) which essentially scales the ray's magntitude
		//NOTE: you can think of similar to normalizing a vector by dividing by magnitude (L2), except using a octahedron equation rather than a sphere (L1)
		float invL1Norm = 1.0f / (abs(normal.m_X) + abs(normal.m_Y) + abs(normal.m_Z));

		//Now we have converted to octahedron, z is reliant on x and y AND THEIR SIGNS
		//since z = 1 - |x| - |y| OR z = -1 + |x| + |y| but now we have to worry about
		//the two possible z values -> whether front or back hemisphere
		Vec2 encoded = normal.GetXY() * invL1Norm;
		//If We are in the front hemisphere (enigne uses z forward as positive)
		//we can just collapse to diamond using x and y coordinates (since z should be positive)
		//OTHERWISE Octahedral folding is required for the back hemisphere (z < 0)
		//which means we take that pyramid and reflect it up.
		//NOTE: it is NOT a regular reflection because then we would get multiple normals with same values,
		//so think of the bottom pyramid as filling the corners of the square with the diamond (front hemisphere) in the center
		if (normal.m_Z < 0.0f)
		{
			encoded = {(1.0f - abs(encoded.m_Y)) * (encoded.m_X >= 0.0f ? 1.0f : -1.0f),
					   (1.0f - abs(encoded.m_X)) * (encoded.m_Y >= 0.0f ? 1.0f : -1.0f)};
		}

		//Now that we encoded into a 2d diamond which fits into x[-1,1] y[-1,1] we map to square -> [0,1]
		encoded = encoded * 0.5f + Vec2(0.5f, 0.5f);
		return encoded;
	}
	Vec3 OctahedralDecodeNormal(const Vec2& octahedral)
	{
		//We remap [0,1] for x and y to [-1,1]
		const Vec2 neg1To1Range = (octahedral - Vec2(0.5f, 0.5f)) * 2.0f;
		//We get z back by solving for z in the octahedral equation: |x| + |y| + |z| = 1 -> z = 1 - |x| - |y| OR z = -1 + |x| + |y|
		Vec3 decoded = Vec3(neg1To1Range.m_X, neg1To1Range.m_Y, 1.0f - abs(neg1To1Range.m_X) - abs(neg1To1Range.m_Y));
		//We do the same operation as for encoding to separate +z and -z
		if (decoded.m_Z < 0.0f)
		{
			decoded.SetXY({(1.0f - abs(decoded.m_Y)) * (decoded.m_X >= 0.0f ? 1.0f : -1.0f),
						   (1.0f - abs(decoded.m_X)) * (decoded.m_Y >= 0.0f ? 1.0f : -1.0f)});
		}
		return decoded.GetNormalized();
	}

	Vec3 ReflectAcrossNormal(const Vec3& incidenceVec, const Vec3& normal)
	{
		//Reflection across normal -> incidenceVec(I) component parallel to normal(N) flips sign, perpendicular component stays same
		//I Parallel component: projection of I onto N -> dotProduct(I,N) * N, Perndicular Component: I - parallelComponent
		//Reflection: -Dot(I,N) * N + I + Dot(I,N) * N = I - 2 * Dot(N, I)
		return incidenceVec - 2 * DotProduct(normal, incidenceVec) * normal;
	}
}