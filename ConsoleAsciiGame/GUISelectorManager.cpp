#include "pch.hpp"
#include "GUISelectorManager.hpp"

#include "InputField.hpp"
#include "GUIRect.hpp"
#include "Debug.hpp"

//TODO: these should all be changed to be parsed or retrieved from ui layer of input profile from input manager
static constexpr MouseButton SELECT_KEY = MOUSE_BUTTON_LEFT;

GUISelectorManager::GUISelectorManager(const Input::InputManager& input) 
	: m_inputManager(input), m_selectables(), m_currentSelected(nullptr), m_currentDragged(nullptr),
	m_selectedThisFrame(false), m_lastFrameMousePos(m_inputManager.GetMousePosition())
{

}

void GUISelectorManager::SelectNewSelectable(SelectableGUI* selectable)
{
	if (selectable == nullptr) return;
	selectable->Select();
	//Assert(false, std::format("SHOULD SELECTE NEW ONE AT: {}", selectable->GetLastFrameRect().ToString()));
}

void GUISelectorManager::ClickSelectable(SelectableGUI* selectable)
{
	//Assert(false, std::format("CLICKIGN ON SELECTABLE"));
	if (selectable == nullptr) return;
	selectable->Click();
}

void GUISelectorManager::DeselectCurrentSelectable()
{
	if (!HasSelecatbleSelected()) return;
	
	m_currentSelected->Deselect();
}

void GUISelectorManager::Update()
{
	//std::string allRect = "";
	//for (auto& selectable : m_selectables)
	//{
	//	if (selectable == nullptr) continue;

	//	allRect += selectable->GetLastFrameRect().ToString();
	//	//selectable->GetLastFrameRect();

	//	//allRect += "HELLO";
	//}


	if (m_currentSelected != nullptr && m_selectedThisFrame) m_selectedThisFrame = false;
	const Input::KeyState selectKeyState = m_inputManager.GetInputKey(SELECT_KEY)->GetState().GetState();

	const Vec2 mousePos = m_inputManager.GetMousePosition();
	const Vec2 mouseDelta = mousePos - m_lastFrameMousePos;
	m_lastFrameMousePos = mousePos;

	//Note: since the initial click on a draggable selectable is the only thing that matters, we do not care if it moves outside the area
	//so we can easily just update the current one that is dragged
	if (selectKeyState == Input::KeyState::Down && m_currentDragged != nullptr)
	{
		LogError(std::format("Updating drag for:{} all selectable:{}", typeid(*m_currentDragged).name(), ToStringSelectableTypes()));
		m_currentDragged->UpdateDrag(mouseDelta, m_inputManager.GetInputKey(SELECT_KEY)->GetState().GetCurrentDownTime());
	}
		
	//NOTE: if the key is not released or pressed, it means no select or drag changes have occured so we do not need to look
	if (m_selectables.empty() || (selectKeyState!= Input::KeyState::Released && selectKeyState!= Input::KeyState::Pressed)) return;

	/*Assert(false, std::format("CLICKED POS:{} FOUNDselectable rect: {} size: {} selected: {}",
			m_lastFrameClickedPosition.value().ToString(), allRect, std::to_string(m_selectables.size()),
			HasSelecatbleSelected() ? m_currentSelected->GetLastFrameRect().ToString() : "NONE"));*/
	
	for (auto& selectable : m_selectables)
	{
		//std::to_string(selectable->GetLastFrameRect().ContainsPos(mousePos))
		if (selectable.second == nullptr) continue;
		/*allRect += std::format("Mouse pos: {} selectable null: {} rect: {}", m_lastFrameClickedPosition.value().ToString(),
			std::to_string(selectable != nullptr), selectable != nullptr ? selectable->GetLastFrameRect().ToString() : "NULL");*/

		
		//NOTE: since we can only click or release on one selectable, we can then break when we find it
		if (selectable.second->GetLastFrameRect().ContainsPos(mousePos))
		{
			if (selectKeyState == Input::KeyState::Pressed)
			{
				//Note: we do not need to cancel the selectable because we can have one selected
				//while we drag on another
				m_currentDragged = selectable.second;
				break;
			}
			else if (selectKeyState == Input::KeyState::Released)
			{
				ClickSelectable(selectable.second);
				SelectNewSelectable(selectable.second);
				m_selectedThisFrame = true;
				break;
			}

			/*Assert(false, std::format("Mouse pos: {} selectable null: {} rect: {}", mousePos.ToString(),
				std::to_string(selectable != nullptr), selectable != nullptr ? selectable->GetLastFrameRect().ToString() : "NULL"));*/
				//LogError(std::format("CLICKED ON NEW SELECTABLE"));
				//Assert(false, std::format("CLICKED ON NEW SELECTABLE: {}", selectable->GetLastFrameRect().ToString()));
			
		}
	}
	//Assert(false, std::format("All rects: {}", allRect));

	

	/*LogError(std::format("Selector has: {} selectables active type: {} (rect: {}) allrect: {}", m_selectables.size(), m_currentSelected != nullptr ?
		ToString(dynamic_cast<InputField*>(m_currentSelected)->GetFieldType()) : "NULL", 
		m_currentSelected!=nullptr? m_currentSelected->GetLastFrameRect().ToString() : "NULL", allRect));*/
}

void GUISelectorManager::AddSelectable(SelectableGUI* selectable, const GUIEventPriority eventPriority)
{
	if (selectable == nullptr) return;

	m_selectables.emplace(eventPriority, selectable);

	//Since we need callback from select and deselect (since it may be called outside of this class)
	//in order to control selection flow, but must prevent recursive calls
	selectable->m_OnSelect.AddListener([this](SelectableGUI* selected)-> void 
		{
			DeselectCurrentSelectable();
			//Note: when the current selected triggers on key released, it means
			//any dragging must have ended
			m_currentSelected = selected;
			m_currentDragged = nullptr;
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

bool GUISelectorManager::HasSelecatbleSelected() const { return m_currentSelected != nullptr; }
const SelectableGUI* GUISelectorManager::TryGetSelectableSelected() const { return m_currentSelected; }
bool GUISelectorManager::SelectedSelectableThisFrame() const { return m_selectedThisFrame; }

std::string GUISelectorManager::ToStringSelectableTypes() const
{
	std::vector<std::string> typeStr = {};
	for (const auto& selectable : m_selectables)
	{
		typeStr.push_back(typeid(selectable.second).name());
	}
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(typeStr);
}
