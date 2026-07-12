#pragma once
#include "Utils/UnitTest.hpp"

#ifdef ENGINE_DEBUG

namespace Engine::Rendering { class Renderer; }
namespace Engine::UnitTests
{
	bool TestBVHIntersectionSphere(Rendering::Renderer& renderer);
}

#endif