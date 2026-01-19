#pragma once
#include "Utils/Math/Vec2Type.hpp"

//A position relative to the screen for raylib of the form [X, Y] where (0,0) is top left
//and bottom right is the canvas (WIDTH, HEIGHT)
using ScreenPosition = Vec2;

constexpr ScreenPosition INVALID_SCREEN_POS = {-1, -1};