#pragma once
#include <string>
#include "Point2DInt.hpp"
#include "Point2D.hpp"
#include "raylib.h"
#include <cstdint>

//TODO: this might have to become a txt or json file
inline const std::string WINDOW_NAME = "game";
constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;
inline const Utils::Point2D SCREEN_ASPECT_RATIO = { 4, 3 };

constexpr float GRAVITY = 12;

//constexpr int TEXT_BUFFER_MAX_WIDTH = 60;
//constexpr int TEXT_BUFFER_MAX_HEIGHT = 30;
//
//constexpr int PLAY_AREA_TEXT_BUFFER_WIDTH = 50;
//constexpr int PLAY_AREA_TEXT_BUFFER_HEIGHT = 25;
//
//inline const Utils::Point2DInt TEXT_BUFFER_PADDING = { 10, 10 };

constexpr float DEBUG_INFO_FONT_SIZE = 15;
inline const Utils::Point2D DEBUG_INFO_CHAR_SPACING = { 3, 2 };

inline const Utils::Point2D GLOBAL_CHAR_AREA = { 2, 2 };
constexpr float GLOBAL_FONT_SIZE = 2;
inline const Utils::Point2D GLOBAL_CHAR_SPACING = Utils::Point2D(1,1);

inline const Color COLLIDER_OUTLINE_COLOR = GREEN;
inline const Color LINE_COLOR = RED;

inline const Color DEBUG_TEXT_COLOR = WHITE;
inline const Color DEBUG_HIGHLIGHTED_TEXT_COLOR = YELLOW;

const std::string MAIN_INPUT_PROFILE_NAME = "main";
const std::string MAIN_INPUT_PROFILE_MOVE_ACTION = "Move";

//Will get the global font. This is needed so that the first call to it
//is guaranteed to create a valid font object rather than pre-initialize with invalid font
//NOTE: this can only be called after Raylib InitWindow() 
Font& GetGlobalFont();


inline const char* ANSI_COLOR_RED = "\033[1;31m";
inline const char* ANSI_COLOR_YELLOW = "\033[1;33m";
inline const char* ANSI_COLOR_WHITE = "\033[1;37m";
inline const char* ANSI_COLOR_GRAY = "\033[1;90m";
inline const char* ANSI_COLOR_GREEN = "\033[1;32m";
inline const char* ANSI_COLOR_CLEAR = "\033[0m";
