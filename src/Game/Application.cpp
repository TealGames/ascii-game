#include "pch.hpp"

#include "Core/Engine.hpp"

#ifdef ENABLE_MEMORY_LEAK_DETECTION
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#endif

int main() 
{
#ifdef ENABLE_MEMORY_LEAK_DETECTION
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    Engine::Core::Engine engine;
    engine.BeginUpdateLoop();
}
