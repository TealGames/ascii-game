#pragma once
#include "RenderInfo.hpp"
#include "Event.hpp"

/// <summary>
/// Represents an interface for a type that does not get rendered immediately but sets
/// up an event with all actions for that render. Useful for measuing the render size before
/// actually rendering it
/// </summary>
struct IDelayedRenderable
{
	virtual ScreenPosition SetupRender(const RenderInfo& renderInfo, Event<void>& renderActions) = 0;
	virtual ~IDelayedRenderable() = default;
};

