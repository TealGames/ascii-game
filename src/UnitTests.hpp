#pragma once
#include "Utils/UnitTest.hpp"

#ifdef ENGINE_DEBUG

namespace Rendering { class Renderer; }
bool TestBVHIntersection(Rendering::Renderer& renderer);

#endif