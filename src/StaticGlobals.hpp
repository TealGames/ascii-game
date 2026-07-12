#pragma once
#include <string>
#include <cstdint>
#include "Core/Primitives/Vector.hpp"

namespace Engine
{
	//TODO: this might have to become a txt or json file
	inline const char* WINDOW_NAME = "game";
	static constexpr std::uint8_t TARGET_FPS = 60;
	inline constexpr int SCREEN_WIDTH = 800;
	inline constexpr int SCREEN_HEIGHT = 600;
	inline constexpr float SCREEN_ASPECT_RATIO = SCREEN_WIDTH / SCREEN_HEIGHT;

	inline constexpr float GRAVITY = 12;

	inline constexpr float DEBUG_INFO_FONT_SIZE = 15;
	inline constexpr Vec2 DEBUG_INFO_CHAR_SPACING = { 3, 2 };

	inline constexpr Vec2 GLOBAL_CHAR_AREA = { 1, 1 };
	inline constexpr Vec2 GLOBAL_FONT_SIZE = { 1, 1 };
	inline constexpr Vec2 GLOBAL_FONT_CHAR_SPACING = { 0.5f, 0.5f };

	inline const char* MAIN_INPUT_PROFILE_NAME = "main";
	inline const char* MAIN_INPUT_PROFILE_MOVE_ACTION = "Move";

#define ASSET_DIR CURRENT_SOURCE_DIR "assets/"
#define MATERIAL_ASSET_DIR ASSET_DIR "materials/"
#define SCENE_ASSET_DIR ASSET_DIR "scenes/"
}


