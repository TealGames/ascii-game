#include "pch.hpp"
#include "GUISelectorManager.hpp"

#include "InputField.hpp"
#include "GUIRect.hpp"
#include "Debug.hpp"

//TODO: these should all be changed to be parsed or retrieved from ui layer of input profile from input manager
static constexpr KeyboardKey SUBMIT_KEY = KEY_ENTER;
static constexpr KeyboardKey ESCAPE_KEY = KEY_E;
static constexpr KeyboardKey DELETE_KEY = KEY_BACKSPACE;
static constexpr MouseButton SELECT_KEY = MOUSE_BUTTON_LEFT;

GUISelectorManager::GUISelectorManager(const Input::InputManager& input) 
	: m_inputManager(input), m_selectables(), m_currentSelected(nullptr) 
{

}

void GUISelectorManager::SelectNewSelectable(SelectableGUI* selectable)
{
	if (selectable == nullptr) return;

	selectable->Select();
	//Assert(false, std::format("SHOULD SELECTE NEW ONE AT: {}", selectable->GetLastFrameRect().ToString()));
}

void GUISelectorManager::DeselectCurrentSelectable()
{
	if (!HasSelecatbleSelected()) return;
	
	m_currentSelected->Deselect();
}

void GUISelectorManager::Update()
{
	std::string allRect = "";
	for (auto& selectable : m_selectables)
	{
		if (selectable == nullptr) continue;

		allRect += selectable->GetLastFrameRect().ToString();
		//selectable->GetLastFrameRect();

		//allRect += "HELLO";
	}

	if (!m_selectables.empty() && m_inputManager.GetInputKey(SELECT_KEY)->GetState().IsReleased())
	{
		ScreenPosition mousePos = m_inputManager.GetMousePosition();

		for (auto& selectable : m_selectables)
		{
			//std::to_string(selectable->GetLastFrameRect().ContainsPos(mousePos))
			if (selectable == nullptr) continue;
			allRect += std::format("Mouse pos: {} selectable null: {} rect: {}", mousePos.ToString(),
				std::to_string(selectable != nullptr), selectable != nullptr ? selectable->GetLastFrameRect().ToString() : "NULL");
			
			if (selectable->GetLastFrameRect().ContainsPos(mousePos))
			{
				SelectNewSelectable(selectable);
				/*Assert(false, std::format("Mouse pos: {} selectable null: {} rect: {}", mousePos.ToString(),
					std::to_string(selectable != nullptr), selectable != nullptr ? selectable->GetLastFrameRect().ToString() : "NULL"));*/
				//LogError(std::format("CLICKED ON NEW SELECTABLE"));
				//Assert(false, std::format("CLICKED ON NEW SELECTABLE: {}", selectable->GetLastFrameRect().ToString()));
				break;
			}
		}
		//Assert(false, std::format("All rects: {}", allRect));
	}

	LogError(std::format("FOUND selectable rect: {} size: {} selected: {}", allRect, std::to_string(m_selectables.size()), 
		HasSelecatbleSelected()? m_currentSelected->GetLastFrameRect().ToString() : "NONE"));

	/*LogError(std::format("Selector has: {} selectables active type: {} (rect: {}) allrect: {}", m_selectables.size(), m_currentSelected != nullptr ?
		ToString(dynamic_cast<InputField*>(m_currentSelected)->GetFieldType()) : "NULL", 
		m_currentSelected!=nullptr? m_currentSelected->GetLastFrameRect().ToString() : "NULL", allRect));*/
}

void GUISelectorManager::AddSelectable(SelectableGUI* selectable)
{
	if (selectable == nullptr) return;

	m_selectables.push_back(selectable);

	//TODO: there is a recursive problem here where if we select/deselct in this class it will invoke event, then 
	//cause the same function to be called then event again, etc.
	selectable->m_OnSelect.AddListener([this](SelectableGUI* selected)-> void 
		{
			//SelectNewSelectable(gui); 
			DeselectCurrentSelectable();
			m_currentSelected = selected;
		});

	selectable->m_OnDeselect.AddListener([this](SelectableGUI* deselected)-> void 
		{
			m_currentSelected = nullptr;
			//DeselectCurrentSelectable(); 
		});

	//We do this check in case it is already selected before it was added to the lsit (which could be the case if selected
	//on start) and we then set it to be selectable if no others are. If there is already one selected (maybe there are multiple
	//selects on start or we add on selected during runtime) then we have to deselect old one to allow incomding process/field to work
	if (selectable->IsSelected())
	{
		if (HasSelecatbleSelected()) DeselectCurrentSelectable();
		else m_currentSelected = selectable;
	}

	/*Assert(false, std::format("Adding selectable: {} Address:{}", 
		m_selectables.back()->GetLastFrameRect().ToString(),  Utils::ToStringPointerAddress(selectable)));*/
}

bool GUISelectorManager::HasSelecatbleSelected() const
{
	return m_currentSelected != nullptr;
}
const SelectableGUI* GUISelectorManager::TryGetSelectableSelected() const
{
	return m_currentSelected;
}
