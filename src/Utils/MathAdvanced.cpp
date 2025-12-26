#include "Utils/MathAdvanced.hpp"
#include "Utils/Math.hpp"

namespace Utils
{
	bool RayIntersectsBoundsInverseDir(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& inverseRayDir, float* outTEnter, float* outTExit)
	{
		//NOTE: we use inverse dir since multiply is faster than divide
		Vec3 tMin = (bounds.m_MinPos - rayOrigin) * inverseRayDir;
		Vec3 tMax = (bounds.m_MaxPos - rayOrigin) * inverseRayDir;

		if (inverseRayDir.m_X < 0) std::swap(tMin.m_X, tMax.m_X);
		if (inverseRayDir.m_Y < 0) std::swap(tMin.m_Y, tMax.m_Y);
		if (inverseRayDir.m_Z < 0) std::swap(tMin.m_Z, tMax.m_Z);

		float tEnter = tMin.GetMaxComponentValue();
		if (outTEnter != nullptr) *outTEnter = tEnter;

		float tExit = tMax.GetMinComponentValue();
		if (outTExit != nullptr) *outTExit = tExit;

		return tExit >= std::max(tEnter, 0.0f);
	}
	bool RayIntersectsBoundsInverseDir(const AABB3D& bounds, const Vec3& rayOrigin, const Vec3& inverseRayDir, float* outMargin)
	{
		float tEnter = 0;
		float tExit = 0;
		bool intersects = RayIntersectsBoundsInverseDir(bounds, rayOrigin, inverseRayDir, &tEnter, &tExit);
		if (outMargin != nullptr)
			*outMargin = tExit - tEnter;

		return intersects;
	}
	bool RayIntersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2, const Vec3& rayOrigin, Vec3 rayDir, float* outHitDistance)
	{
		rayDir = rayDir.GetNormalized();

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

		if (outHitDistance != nullptr)
			*outHitDistance = t;
		return true;
	}
	bool RayIntersectsSphere(const Vec3& sphereCenter, const float radius, const Vec3& rayOrigin, const Vec3& rayDir, float* outHitDistance)
	{
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
			if (outHitDistance != nullptr) *outHitDistance = t0;
			return true;
		}
		const float t1 = -tca + thc;
		if (t1 > 0)
		{
			if (outHitDistance != nullptr) *outHitDistance = t1;
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
		return CalculateModelMatrix(parentMatrix, CalculateTranslationMatrix(pos), 
			CalculateScaleMatrix(scale), CalculateRotationMatrix(rotation));
	}
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Vec3& pos, const Vec3& scale, const Mat4& rotation)
	{
		return CalculateModelMatrix(parentMatrix, CalculateTranslationMatrix(pos),
			CalculateScaleMatrix(scale), rotation);
	}
	Mat4 CalculateModelMatrix(const Mat4* parentMatrix, const Mat4& posMatrix, const Mat4& scaleMatrix, const Mat4& rotationMatrix)
	{
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
}