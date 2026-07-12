#pragma once
#include "Core/Primitives/Color.hpp"
#include "Fig/Fig.hpp"

namespace Engine::Serialization
{
	ColHDR4 ToColor(const std::string& value);
}