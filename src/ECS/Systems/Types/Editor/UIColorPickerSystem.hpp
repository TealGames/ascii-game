#pragma once

namespace Engine::UI { class PopupUIManager; }
namespace Engine::Editor::UI
{
	namespace UI = Engine::UI;

	class UIColorPickerSystem
	{
	private:
		UI::PopupUIManager* m_popupManager;
	public:

	private:
	public:
		UIColorPickerSystem(UI::PopupUIManager& popupManager);
		void Init();
	};
}