#include "pch.hpp"
#include "GUISelectorManager.hpp"
#include "GUIRect.hpp"
#include "Debug.hpp"

//TODO: these should all be changed to be parsed or retrieved from ui layer of input profile from input manager
static constexpr MouseButton SELECT_KEY = MOUSE_BUTTON_LEFT;

GUISelectorManager::GUISelectorManager(const Input::InputManager& input, GUIHierarchy& hierarchy)
	: m_inputManager(input), m_hierarchy(hierarchy), m_selectableLayers(), m_currentSelected(nullptr), m_currentDragged(nullptr),
	m_selectedThisFrame(false), m_lastFrameMousePos(m_inputManager.GetMousePosition())
{
}


void GUISelectorManager::CraeteSelectableArray()
{
	m_selectableLayers.clear();
	m_hierarchy.ExecuteOnAllElementsDescending([this](GUILayer layer, GUIElement* element) -> void
		{
			if (SelectableGUI* selectable = dynamic_cast<SelectableGUI*>(element))
				AddSelectable(layer, selectable);
		});
}

void GUISelectorManager::Init()
{
	CraeteSelectableArray();

	//TODO: this is slow and not very good solution to have to recreate selectable array on every addition/removal
	m_hierarchy.m_OnElementAdded.AddListener([this](const GUIElement* element)-> void 
		{
			CraeteSelectableArray();
		});

	m_hierarchy.m_OnElementRemoved.AddListener([this](const GUIElement* element)-> void
		{
			CraeteSelectableArray();
		});
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
	if (m_selectableLayers.empty() || (selectKeyState!= Input::KeyState::Released && selectKeyState!= Input::KeyState::Pressed)) return;

	/*Assert(false, std::format("CLICKED POS:{} FOUNDselectable rect: {} size: {} selected: {}",
			m_lastFrameClickedPosition.value().ToString(), allRect, std::to_string(m_selectables.size()),
			HasSelecatbleSelected() ? m_currentSelected->GetLastFrameRect().ToString() : "NONE"));*/
	
	for (auto& layer : m_selectableLayers)
	{
		for (auto& selectable : layer.second)
		{
			//std::to_string(selectable->GetLastFrameRect().ContainsPos(mousePos))
			if (selectable == nullptr) continue;
			/*allRect += std::format("Mouse pos: {} selectable null: {} rect: {}", m_lastFrameClickedPosition.value().ToString(),
				std::to_string(selectable != nullptr), selectable != nullptr ? selectable->GetLastFrameRect().ToString() : "NULL");*/


				//NOTE: since we can only click or release on one selectable, we can then break when we find it
			if (selectable->GetLastFrameRect().ContainsPos(mousePos))
			{
				if (selectKeyState == Input::KeyState::Pressed)
				{
					//Note: we do not need to cancel the selectable because we can have one selected
					//while we drag on another
					m_currentDragged = selectable;
					break;
				}
				else if (selectKeyState == Input::KeyState::Released)
				{
					ClickSelectable(selectable);
					SelectNewSelectable(selectable);
					m_selectedThisFrame = true;
					break;
				}

				/*Assert(false, std::format("Mouse pos: {} selectable null: {} rect: {}", mousePos.ToString(),
					std::to_string(selectable != nullptr), selectable != nullptr ? selectable->GetLastFrameRect().ToString() : "NULL"));*/
					//LogError(std::format("CLICKED ON NEW SELECTABLE"));
					//Assert(false, std::format("CLICKED ON NEW SELECTABLE: {}", selectable->GetLastFrameRect().ToString()));

			}
		}
	}
	
	//Assert(false, std::format("All rects: {}", allRect));

	

	/*LogError(std::format("Selector has: {} selectables active type: {} (rect: {}) allrect: {}", m_selectables.size(), m_currentSelected != nullptr ?
		ToString(dynamic_cast<InputField*>(m_currentSelected)->GetFieldType()) : "NULL", 
		m_currentSelected!=nullptr? m_currentSelected->GetLastFrameRect().ToString() : "NULL", allRect));*/
}

void GUISelectorManager::AddSelectable(const GUILayer layer, SelectableGUI* selectable)
{
	auto it = m_selectableLayers.find(layer);

	if (it == m_selectableLayers.end())
		m_selectableLayers.emplace(layer, std::vector<SelectableGUI*>{ selectable});
	else it->second.push_back(selectable);
}
void GUISelectorManager::AddSelectables(const GUILayer layer, const std::vector<SelectableGUI*>& selectables)
{
	for (const auto& selectable : selectables)
		AddSelectable(layer, selectable);
}

bool GUISelectorManager::HasSelecatbleSelected() const { return m_currentSelected != nullptr; }
const SelectableGUI* GUISelectorManager::TryGetSelectableSelected() const { return m_currentSelected; }
bool GUISelectorManager::SelectedSelectableThisFrame() const { return m_selectedThisFrame; }

std::string GUISelectorManager::ToStringSelectableTypes() const
{
	std::vector<std::string> typeStr = {};
	for (auto& layer : m_selectableLayers)
	{
		for (const auto& selectable : layer.second)
		{
			typeStr.push_back(typeid(selectable).name());
		}
	}
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(typeStr);
}
