#pragma once
#include "Core/Visual/VisualData.hpp"
#include "Fig/Fig.hpp"

VisualData ParseDefaultVisualData(const std::vector<std::string>& lines);
VisualData ParseDefaultVisualData(const FigProperty& property);

