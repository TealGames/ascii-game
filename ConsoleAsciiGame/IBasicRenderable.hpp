#pragma once

/// <summary>
/// A basic renderable that does not need any render info or 
/// other data pased in
/// </summary>
struct IBasicRenderable
{
	virtual bool TryRender() = 0;
	virtual ~IBasicRenderable() = default;
};

