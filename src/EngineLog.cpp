#include "EngineLog.hpp"
#include "StaticGlobals.hpp"
#include "AnsiCodes.hpp"

namespace Core
{
	void EngineLog(const std::string& log)
	{
#if defined(ENGINE_LOG)
		LogMessage(LogType::Log, CallerLogDetails::None, log, false, true, ANSI_COLOR_BLUE, false);
#endif
	}
}