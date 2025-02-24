#pragma once

struct ISelectable
{
	virtual void Select() = 0;
	virtual void Deselect() = 0;
};

