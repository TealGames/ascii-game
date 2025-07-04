#pragma once
#include <string>
#include "raylib.h"
#include <cstdint>
#include "Utils/Data/Vec2.hpp"
#include "Utils/Data/Vec2Int.hpp"

//TODO: this might have to become a txt or json file
inline const char* WINDOW_NAME = "game";
static constexpr std::uint8_t TARGET_FPS = 60;
inline constexpr int SCREEN_WIDTH = 800;
inline constexpr int SCREEN_HEIGHT = 600;
inline const Vec2Int SCREEN_ASPECT_RATIO = { 4, 3 };

inline constexpr float GRAVITY = 12;

inline constexpr float DEBUG_INFO_FONT_SIZE = 15;
inline const Vec2 DEBUG_INFO_CHAR_SPACING = { 3, 2 };

inline const Vec2 GLOBAL_CHAR_AREA = { 2, 2 };
inline constexpr float GLOBAL_FONT_SIZE = 2;
inline const Vec2 GLOBAL_CHAR_SPACING = Vec2(1, 1);

inline const char* MAIN_INPUT_PROFILE_NAME = "main";
inline const char* MAIN_INPUT_PROFILE_MOVE_ACTION = "Move";

