#include "UnitTests.hpp"
#include "Core/Rendering/Renderer3d.hpp"

#ifdef ENGINE_DEBUG

bool TestBVHIntersection(Rendering::Renderer& renderer)
{
	//NOTE: because c++ is so shit we must create test func so it does not get 
	//confused with lambda
	std::function<bool(Vec3, Vec3)> testFunc =
		[&renderer](Vec3 rayWorldOrigin, Vec3 rayDir) -> bool
		{
			return renderer.IntersectsBVH(rayWorldOrigin, rayDir, nullptr);
		};

	std::vector<UnitTest::FunctionTest<bool, Vec3, Vec3>> tests =
	{
		{true, Vec3(0, 0, -0.1), Vec3(0, 0, 1)},
		{true, Vec3(0, 0, -0.1), Vec3(0.1, 0, 0.4).GetNormalized()},
		{true, Vec3(0, 0, -0.1), Vec3(0.2, 0, 0.4).GetNormalized()},
		{true, Vec3(0, 0, -0.1), Vec3(0.21, 0, 0.4).GetNormalized()},
		{false, Vec3(0, 0, -0.1), Vec3(0.4, 0, 0.4).GetNormalized()},
		{false, Vec3(0, 0, -0.1), Vec3(0, 0, -1).GetNormalized()}
	};

	return UnitTest::TestFunction("IntersectsBVH", testFunc, tests);
}

#endif
