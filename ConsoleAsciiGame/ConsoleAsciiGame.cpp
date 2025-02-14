//NOT USED
#include "pch.hpp"

#include "Engine.hpp"
#include "raylib.h"
#include <optional>
#include "CommandPromptType.hpp"
#include "Event.hpp"


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

    try
    {
        Core::Engine engine; 
        engine.BeginUpdateLoop();
    }
    catch (const std::exception& e)
    {
        LogError(std::format("Encountered Engine error: {}", e.what()));
        return 0;
    }
}
