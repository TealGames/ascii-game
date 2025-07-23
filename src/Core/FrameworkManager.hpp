#pragma once
#include "Utils/Data/Result.hpp"

namespace Core
{
	BasicResult<bool> FrameworkInit();
	void FrameworkUpdate();
	void FrameworkShutdown();
}