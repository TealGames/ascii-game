#include "pch.hpp"
#include "GUISelectorManager.hpp"
#include "GUIRect.hpp"
#include "Debug.hpp"
#include "HelperFunctions.hpp"

//TODO: these should all be changed to be parsed or retrieved from ui layer of input profile from input manager
static constexpr MouseButton SELECT_KEY = MOUSE_BUTTON_LEFT;

GUISelectorManager::GUISelectorManager(const Input::InputManager& input, GUIHierarchy& hierarchy)
	: m_inputManager(input), m_hierarchy(hierarchy), m_selectableLayers(), m_selectionEventBlockers(), 
	m_currentSelected(nullptr), m_currentDragged(nullptr), m_currentHovered(nullptr),
	m_selectedThisFrame(false), m_hasGuiTreeUpdated(false), m_lastFrameMousePos(m_inputManager.GetMousePosition())
{
}

void GUISelectorManager::CreateSelectableArray()
{
	m_selectableLayers.clear();

	size_t i = 0;
	m_hierarchy.ExecuteOnAllElementsDescending([this, &i](GUILayer layer, GUIElement* element) -> void
		{
			if (element == nullptr) return;
			//LogWarning(std::format("Bool is element:{} selectable:", element->ToStringBase()));
			if (element->IsSelectionEventBlocker()) AddSelectionEventBlocker(layer, i, element);
			else if (SelectableGUI* selectable = dynamic_cast<SelectableGUI*>(element))
			{
				AddSelectable(layer, selectable);
				//LogWarning(std::format("YES"));
			}

			i++;
		});
	//LogError(std::format("END of selectable array craetion:{}", m_hierarchy.ToStringTree()));
}

void GUISelectorManager::TryUpdateTree()
{
	if (!m_hasGuiTreeUpdated) return;

	CreateSelectableArray();
	m_hasGuiTreeUpdated = false;
}

void GUISelectorManager::Init()
{
	CreateSelectableArray();

	//TODO: this is slow and not very good solution to have to recreate selectable array on every addition/removal
	m_hierarchy.m_OnElementAdded.AddListener([this](const GUIElement* element)-> void
		{
			//static size_t i = 0;

			//LogError(std::format("DOPE SHIT"));
			//CraeteSelectableArray();
			m_hasGuiTreeUpdated = true;

			/*if (i>=1) Assert(false, std::format("Updated selectable array ({}) element added:{} selectables:{} HIERARCHY:{}", 
				std::to_string(i), element->ToStringBase(), ToStringSelectables(), m_hierarchy.ToStringTree()));
			i++;*/
		});

	m_hierarchy.m_OnElementRemoved.AddListener([this](const GUIElement* element)-> void
		{
			//CreateSelectableArray();
			m_hasGuiTreeUpdated = true;
		});
}

void GUISelectorManager::SelectNewSelectable(SelectableGUI* selectable)
{
	if (selectable == nullptr) return;
	LogWarning(std::format("SELEC NEW SELECTABLE:{}", selectable->ToStringBase()));

	DeselectCurrentSelectable();
	selectable->Select();
	m_currentSelected = selectable;
	//Assert(false, std::format("SHOULD SELECTE NEW ONE AT: {}", selectable->GetLastFrameRect().ToString()));
}

void GUISelectorManager::ClickSelectable(SelectableGUI* selectable)
{
	//Assert(false, std::format("CLICKIGN ON SELECTABLE"));
	if (selectable == nullptr) return;
	selectable->Click();
	LogWarning(std::format("CLICK SELECTABLE:{} tree:{}", selectable->ToStringBase(), m_hierarchy.ToStringTree()));
	LogError(std::format("click selectable ADDR:{}", Utils::ToStringPointerAddress(selectable)));
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

	TryUpdateTree();
	InvokeInteractionEvents();
	m_lastFrameMousePos = m_inputManager.GetMousePosition();
	
	//Assert(false, std::format("All rects: {}", allRect));

	

	/*LogError(std::format("Selector has: {} selectables active type: {} (rect: {}) allrect: {}", m_selectables.size(), m_currentSelected != nullptr ?
		ToString(dynamic_cast<InputField*>(m_currentSelected)->GetFieldType()) : "NULL", 
		m_currentSelected!=nullptr? m_currentSelected->GetLastFrameRect().ToString() : "NULL", allRect));*/
}

void GUISelectorManager::InvokeInteractionEvents()
{
	static int clickTime = 0;

	if (m_currentSelected != nullptr && m_selectedThisFrame) m_selectedThisFrame = false;
	const Input::KeyState selectKeyState = m_inputManager.GetInputKey(SELECT_KEY)->GetState().GetState();

	const Vec2 mousePos = m_inputManager.GetMousePosition();
	const Vec2 mouseDelta = mousePos - m_lastFrameMousePos;
	//m_lastFrameMousePos = mousePos;

	//Note: since the initial click on a draggable selectable is the only thing that matters, we do not care if it moves outside the area
	//so we can easily just update the current one that is dragged
	if (selectKeyState == Input::KeyState::Down && m_currentDragged != nullptr)
	{
		//LogError(std::format("Updating drag for:{} all selectable:{}", typeid(*m_currentDragged).name(), ToStringSelectableTypes()));
		m_currentDragged->UpdateDrag(mouseDelta, m_inputManager.GetInputKey(SELECT_KEY)->GetState().GetCurrentDownTime());
	}

	const bool hasNewHoverPos = mousePos != m_lastFrameMousePos;
	const bool hasSelectionEvent = selectKeyState == Input::KeyState::Released || selectKeyState == Input::KeyState::Pressed;
	//NOTE: if the key is not released or pressed AND the position has not changed, it means no select, drag or hover changes have occured so we do not need to look
	if (m_selectableLayers.empty() || (!hasSelectionEvent && !hasNewHoverPos)) return;
	

	/*Assert(false, std::format("CLICKED POS:{} FOUNDselectable rect: {} size: {} selected: {}",
			m_lastFrameClickedPosition.value().ToString(), allRect, std::to_string(m_selectables.size()),
			HasSelecatbleSelected() ? m_currentSelected->GetLastFrameRect().ToString() : "NONE"));*/

	bool foundEventBlock = false;
	//Note: we start at -1 because we increment at the start of each iteration due to guard statements
	int i = -1;
	for (auto& layer : m_selectableLayers)
	{
		for (auto& selectable : layer.second)
		{
			i++;
			if (selectable == nullptr)
			{
				auto it = m_selectionEventBlockers.find(i);
				if (it == m_selectionEventBlockers.end()) continue;

				//Note: only if the event blocker contains the position do we block further events
				if (it->second != nullptr && it->second->GetLastFrameRect().ContainsPos(mousePos))
				{
					foundEventBlock = true;
					break;
				}
				else continue;
			}

			/*allRect += std::format("Mouse pos: {} selectable null: {} rect: {}", m_lastFrameClickedPosition.value().ToString(),
				std::to_string(selectable != nullptr), selectable != nullptr ? selectable->GetLastFrameRect().ToString() : "NULL");*/


				//NOTE: since we can only click or release on one selectable, we can then break when we find it
				/*LogError(std::format("checking selectable:{} last rect:{} contains pos:{}->{}", selectable->ToStringBase(),
					selectable->GetLastFrameRect().ToString(), mousePos.ToString(), std::to_string(selectable->GetLastFrameRect().ContainsPos(mousePos))));*/

			if (!selectable->GetLastFrameRect().ContainsPos(mousePos)) continue;

			if (hasNewHoverPos)
			{
				m_currentHovered = selectable;
				if (!hasSelectionEvent) break;
			}

			if (!hasSelectionEvent) continue;

			//Note: from both press and release events we can break without caring about the hover event because
			//both events would be triggered for the same object that should have gotten checked for the hover event few lines above
			//Press triggers a drag event WHICH IS INDEPENDENT FROM A SELECT EVENT
			if (selectKeyState == Input::KeyState::Pressed)
			{
				//Note: we do not need to cancel the selectable because we can have one selected
				//while we drag on another
				m_currentDragged = selectable;
				break;
			}
			else if (selectKeyState == Input::KeyState::Released)
			{
				LogError(std::format("BEFORE click selectable ADDR:{} layer total:{}", Utils::ToStringPointerAddress(selectable), std::to_string(layer.second.size())));
				ClickSelectable(selectable);
				LogError(std::format("AFTER click selectable ADDR:{} layer total:{}", Utils::ToStringPointerAddress(selectable), std::to_string(layer.second.size())));

				SelectNewSelectable(selectable);
				m_selectedThisFrame = true;
				break;
			}

			/*Assert(false, std::format("Mouse pos: {} selectable null: {} rect: {}", mousePos.ToString(),
				std::to_string(selectable != nullptr), selectable != nullptr ? selectable->GetLastFrameRect().ToString() : "NULL"));*/
				//LogError(std::format("CLICKED ON NEW SELECTABLE"));
				//Assert(false, std::format("CLICKED ON NEW SELECTABLE: {}", selectable->GetLastFrameRect().ToString()));
		}

		if (foundEventBlock) break;
	}

	//clickTime++;
	if (selectKeyState == Input::KeyState::Released && clickTime > 2)
		Assert(false, std::format("After event CLICKED:{} \n\n selectavle tree:{} \n\ngui tree:{}", mousePos.ToString(), ToStringSelectables(), m_hierarchy.ToStringTree()));
}

void GUISelectorManager::AddSelectable(const GUILayer layer, SelectableGUI* selectable)
{
	if (selectable == nullptr)
	{
		Assert(false, std::format("Tried to add selectable to gui selector manager but it is NULL"));
		return;
	}

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

void GUISelectorManager::AddSelectionEventBlocker(const GUILayer layer, const size_t elementIndex, const GUIElement* element)
{
	auto it = m_selectableLayers.find(layer);
	//TODO: it might not be a good idea to have nullptr as the placeholder for event blockers in case selectables are null by accident

	//Note: NULLPTR is a placeholder for event blockers
	if (it == m_selectableLayers.end())
		m_selectableLayers.emplace(layer, std::vector<SelectableGUI*>{ nullptr});
	else it->second.push_back(nullptr);

	m_selectionEventBlockers.emplace(elementIndex, element);
}
bool GUISelectorManager::IsEventBlocker(const size_t index) const
{
	return m_selectionEventBlockers.find(index) != m_selectionEventBlockers.end();
}

bool GUISelectorManager::SelectedSelectableThisFrame() const { return m_selectedThisFrame; }
bool GUISelectorManager::HasSelecatbleSelected() const { return m_currentSelected != nullptr; }
const SelectableGUI* GUISelectorManager::TryGetSelectableSelected() const { return m_currentSelected; }
const SelectableGUI* GUISelectorManager::TryGetSelectableDragged() const { return m_currentDragged; }
const SelectableGUI* GUISelectorManager::TryGetSelectableHovered() const { return m_currentHovered; }


std::string GUISelectorManager::ToStringSelectableTypes() const
{
	std::vector<std::string> typeStr = {};
	size_t i = 0;
	for (auto& layer : m_selectableLayers)
	{
		for (const auto& selectable : layer.second)
		{
			if (selectable == nullptr)
			{
				if (IsEventBlocker(i)) typeStr.push_back("[EVENT BLOCKER]");
				else typeStr.push_back("[NULL]");
			}
			else typeStr.push_back(typeid(selectable).name());
			i++;
		}
	}
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(typeStr);
}

std::string GUISelectorManager::ToStringSelectables() const
{
	std::vector<std::string> layersList = {};
	std::vector<std::string> selectabeList = {};

	size_t i = 0;
	for (const auto& layer : m_selectableLayers)
	{
		layersList.push_back("[Layer, Selectables:");
		for (const auto& selectable : layer.second)
		{
			if (selectable == nullptr)
			{
				if (IsEventBlocker(i)) selectabeList.push_back("[EVENT BLOCKER]");
				else selectabeList.push_back("[NULL]");
			}
			else selectabeList.push_back(selectable->ToStringBase());
		}
		layersList.back() += Utils::ToStringIterable(selectabeList);
		layersList.back().push_back(']');
	}
	return Utils::ToStringIterable(layersList);
}
