#include "Utils/MathAdvanced.hpp"
#include "Utils/Math.hpp"

namespace Utils
{
	bool RayIntersectsBounds(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& rayDir, float* outTEnter, float* outTExit)
	{
		ENGINE_ASSERT(rayDir.IsUnitVector(), "Attempted to invoke RayIntersectsBounds but ray dir is not unit vector:{}", rayDir.ToString());
		Vec3 tMin, tMax;
		// X axis
		if (fabs(rayDir.m_X) < EPSILON)
		{
			// Ray is parallel and outside the slab
			if (rayOrigin.m_X < bounds.m_MinPos.m_X || rayOrigin.m_X > bounds.m_MaxPos.m_X)
				return false; 
			tMin.m_X = -std::numeric_limits<float>::infinity();
			tMax.m_X = std::numeric_limits<float>::infinity();
		}
		else 
		{
			float inv = 1.0f / rayDir.m_X;
			tMin.m_X = (bounds.m_MinPos.m_X - rayOrigin.m_X) * inv;
			tMax.m_X = (bounds.m_MaxPos.m_X - rayOrigin.m_X) * inv;
			if (tMin.m_X > tMax.m_X) std::swap(tMin.m_X, tMax.m_X);
		}

		// Y axis
		if (fabs(rayDir.m_Y) < EPSILON)
		{
			if (rayOrigin.m_Y < bounds.m_MinPos.m_Y || rayOrigin.m_Y > bounds.m_MaxPos.m_Y)
				return false;
			tMin.m_Y = -std::numeric_limits<float>::infinity();
			tMax.m_Y = std::numeric_limits<float>::infinity();
		}
		else 
		{
			float inv = 1.0f / rayDir.m_Y;
			tMin.m_Y = (bounds.m_MinPos.m_Y - rayOrigin.m_Y) * inv;
			tMax.m_Y = (bounds.m_MaxPos.m_Y - rayOrigin.m_Y) * inv;
			if (tMin.m_Y > tMax.m_Y) std::swap(tMin.m_Y, tMax.m_Y);
		}

		// Z axis
		if (fabs(rayDir.m_Z) < EPSILON)
		{
			if (rayOrigin.m_Z < bounds.m_MinPos.m_Z || rayOrigin.m_Z > bounds.m_MaxPos.m_Z)
				return false;
			tMin.m_Z = -std::numeric_limits<float>::infinity();
			tMax.m_Z = std::numeric_limits<float>::infinity();
		}
		else 
		{
			float inv = 1.0f / rayDir.m_Z;
			tMin.m_Z = (bounds.m_MinPos.m_Z - rayOrigin.m_Z) * inv;
			tMax.m_Z = (bounds.m_MaxPos.m_Z - rayOrigin.m_Z) * inv;
			if (tMin.m_Z > tMax.m_Z) std::swap(tMin.m_Z, tMax.m_Z);
		}

		float tEnter = tMin.GetMaxComponentValue();
		float tExit = tMax.GetMinComponentValue();

		if (outTEnter) *outTEnter = tEnter;
		if (outTExit)  *outTExit = tExit;

		return tExit >= std::max(tEnter, 0.0f);
	}
	bool RayIntersectsBounds(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& rayDir, float* outMargin)
	{
		float tEnter = 0;
		float tExit = 0;
		bool intersects = RayIntersectsBounds(bounds, rayOrigin, rayDir, &tEnter, &tExit);
		if (outMargin != nullptr)
			*outMargin = tExit - tEnter;

		return intersects;
	}
	bool RayIntersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& rayOrigin, const Vec3& rayDir, float* outTEnter)
	{
		ENGINE_ASSERT(rayDir.IsUnitVector(), "Attempted to invoke RayIntersectsTriangle but ray dir is not unit vector:{}", rayDir.ToString());

		const Vec3 e1 = v1 - v0;
		const Vec3 e2 = v2 - v0;
		const Vec3 p = CrossProduct(rayDir, e2);
		const float determinant = DotProduct(e1, p);
		if (fabs(determinant) < EPSILON)
			return false;

		const float inverseDeterminant = 1.0f / determinant;
		const Vec3 tvec = rayOrigin - v0;
		const float u = DotProduct(tvec, p) * inverseDeterminant;
		if (u < 0 || u > 1)
			return false;

		const Vec3 q = CrossProduct(tvec, e1);
		const float v = DotProduct(rayDir, q) * inverseDeterminant;
		if (v < 0 || u + v > 1)
			return false;

		const float t = DotProduct(e2, q) * inverseDeterminant;
		if (t < 0)
			return false;

		if (outTEnter != nullptr)
			*outTEnter = t;
		return true;
	}
	bool RayIntersectsSphere(const Vec3& sphereCenter, const float radius, const Vec3& rayOrigin, const Vec3& rayDir, float* outTEnter)
	{
		ENGINE_ASSERT(rayDir.IsUnitVector(), "Attempted to invoke RayIntersectsSphere but ray dir is not unit vector:{}", rayDir.ToString());

		//NOTE: the ray dir does not have to be normalized because it is just 
		//used in the equation of a 3d line <x,y,z> = Vo + Vt where V is the non-unit vector direction
		const Vec3 l = rayOrigin - sphereCenter;
		const float tca = DotProduct(l, rayDir);
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

	bool IsWithinBounds(const AABB3D& aabb, const Vec3& pos)
	{
		if (pos.AnyAxisGreaterThan(aabb.m_MaxPos))
			return false;
		if (pos.AnyAxisLessThan(aabb.m_MinPos))
			return false;

		return true;
	}

	bool IsFullyOutsideBounds(const AABB3D& aabb, const Vec3& pos)
	{
		if (pos > aabb.m_MinPos && pos > aabb.m_MaxPos)
			return true;
		if (pos < aabb.m_MinPos && pos < aabb.m_MaxPos)
			return true;

		return false;
	}

	float CalculateSurfaceArea(const Vec3& minPos, const Vec3& maxPos)
	{
		const float xDiff = maxPos.m_X - minPos.m_X;
		const float yDiff = maxPos.m_Y - minPos.m_Y;
		const float zDiff = maxPos.m_Z - minPos.m_Z;
		return 2 * (xDiff * yDiff + yDiff * zDiff + xDiff * zDiff);
	}
	float CalculateSurfaceArea(const AABB3D& bounds)
	{
		return CalculateSurfaceArea(bounds.m_MinPos, bounds.m_MaxPos);
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

		return Mat4(
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
}