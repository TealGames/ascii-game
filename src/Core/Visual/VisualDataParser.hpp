#pragma once
#include "Core/Visual/VisualData.hpp"
#include "Fig/Fig.hpp"

namespace Engine::Rendering
{
	VisualData ParseDefaultVisualData(const std::vector<std::string>& lines);
	VisualData ParseDefaultVisualData(const FigFormat::FigProperty& property);
}


