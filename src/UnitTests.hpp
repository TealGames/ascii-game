#pragma once
#include "Utils/UnitTest.hpp"

#ifdef ENGINE_DEBUG

namespace Rendering { class Renderer; }
bool TestBVHIntersectionSphere(Rendering::Renderer& renderer);
#endif