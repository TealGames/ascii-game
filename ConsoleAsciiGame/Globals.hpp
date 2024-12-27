#pragma once
#include <string>
#include "Point2DInt.hpp"

//TODO: this might have to become a txt or json file
const std::string WINDOW_NAME = "game";
constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 600;

constexpr int TEXT_BUFFER_MAX_WIDTH = 60;
constexpr int TEXT_BUFFER_MAX_HEIGHT = 30;

constexpr int PLAY_AREA_TEXT_BUFFER_WIDTH = 50;
constexpr int PLAY_AREA_TEXT_BUFFER_HEIGHT = 25;
const Utils::Point2DInt CHAR_SPACING = Utils::Point2DInt(5, 5);

constexpr int TEXT_BUFFER_FONT = 15;
const Utils::Point2DInt TEXT_BUFFER_PADDING = { 10, 10 };