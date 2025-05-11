#pragma once
#include "GUIElement.hpp"

struct ITreeGUIConstructible
{
	~ITreeGUIConstructible() = default;
	virtual GUIElement* GetTreeGUI() = 0;
};

