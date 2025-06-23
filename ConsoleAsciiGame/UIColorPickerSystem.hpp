#pragma once
#include "UIColorPicker.hpp"

class PopupUIManager;
namespace ECS
{
	class UIColorPickerSystem
	{
	private:
		PopupUIManager* m_popupManager;
	public:

	private:
	public:
		UIColorPickerSystem(PopupUIManager& popupManager);
	};
}