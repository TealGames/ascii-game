#pragma once
#include <string>
#include "Utils/Debug.hpp"
#include "StaticGlobals.hpp"
#include "Utils/AnsiCodes.hpp"

namespace Engine::Core
{
	template<typename... Args>
	void EngineLog(const char* message, Args&&... args)
	{
#if ENGINE_LOG
		LogMessage(LogType::Log, CallerLogDetails::None, 
			InvokeFormatVariadicArgs<Args...>(message, std::forward<Args>(args)...), 
			false, true, ANSI_COLOR_BLUE, false);
#endif
	}
}
