#pragma once
struct ILoadable
{
	virtual ~ILoadable() = default;

	virtual void Load() = 0;
	virtual void Unload() = 0;
};

