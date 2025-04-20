//NOT USED
#include "pch.hpp"

#include "Engine.hpp"
#include "raylib.h"
#include "PreservedMap.hpp"
#include "Debug.hpp"

#ifdef ENABLE_MEMORY_LEAK_DETECTION
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif

int main() {

#ifdef ENABLE_MEMORY_LEAK_DETECTION
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    //std::optional<float> maybeFloat = TryConvertPromptArgToType<float>("0");
    //Log(std::format("Maybe float val: {}", std::to_string(maybeFloat.value() + 1)));
    //throw std::invalid_argument("");

  /* PreservedMap<std::string, bool> map = {};
    auto p1= map.Insert("fart", true);
    LogError(std::format("P1:{}", p1.first.ToString()));
    auto p2= map.Insert("shit", false);
    LogError(std::format("P2:{}", p2.first.ToString()));
    auto p3= map.Insert("piss", true);
    LogError(std::format("P3:{}", p3.first.ToString()));
    auto p4= map.Insert("poop", false);
    LogError(std::format("P4:{}", p4.first.ToString()));
    map.Insert("yeah", true);
    map.Insert("hi", true);
    map.Insert("tide", true);
    map.Insert("dick", true);
    map.Insert("sack", true);
    LogError(std::format("Values:{}", map.ToString()));
    LogError(std::format("Hello: {}", std::to_string(map.FindMutable("shit").GetValue())));
    return 0;*/

    /*
    std::unordered_map<std::string, bool> map2 = {};
    map2.emplace("fart", true);
    map2.emplace("shit", false);
    map2.emplace("piss", true);
    map2.emplace("poop", false);
    map2.emplace("yeah", true);
    map2.emplace("hi", true);
    map2.emplace("tide", true);
    map2.emplace("dick", true);
    map2.emplace("sack", true);
    for (auto it = map2.begin(); it != map2.end(); it++)
    {
        LogError(std::format("Found:{}", it->first));
    }
    */

    try
    {
        InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_NAME.c_str());
        SetTargetFPS(TARGET_FPS);

        Core::Engine engine; 
        engine.BeginUpdateLoop();
    }
    catch (const std::exception& e)
    {
        LogError(std::format("Encountered Engine error: {}", e.what()));
        return 0;
    }
}
