#include "pch.hpp"
#include "GUISelectorManager.hpp"
#include "GUIRect.hpp"
#include "Debug.hpp"
#include "HelperFunctions.hpp"

//TODO: these should all be changed to be parsed or retrieved from ui layer of input profile from input manager
static constexpr MouseButton SELECT_KEY = MOUSE_BUTTON_LEFT;

GUISelectorManager::GUISelectorManager(const Input::InputManager& input, GUIHierarchy& hierarchy)
	: m_inputManager(input), m_hierarchy(hierarchy), m_selectableLayers(), m_currentSelected(nullptr), m_currentDragged(nullptr),
	m_selectedThisFrame(false), m_guiTreeUpdated(false), m_lastFrameMousePos(m_inputManager.GetMousePosition())
{
}

void GUISelectorManager::CreateSelectableArray()
{
	m_selectableLayers.clear();
	m_hierarchy.ExecuteOnAllElementsDescending([this](GUILayer layer, GUIElement* element) -> void
		{
			if (element == nullptr) return;
			//LogWarning(std::format("Bool is element:{} selectable:", element->ToStringBase()));
			if (SelectableGUI* selectable = dynamic_cast<SelectableGUI*>(element))
			{
				AddSelectable(layer, selectable);
				//LogWarning(std::format("YES"));
			}
		});
	LogError(std::format("END of selectable array craetion:{}", m_hierarchy.ToStringTree()));
}

void GUISelectorManager::TryUpdateTree()
{
	if (!m_guiTreeUpdated) return;

	CreateSelectableArray();
	m_guiTreeUpdated = false;
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
			m_guiTreeUpdated = true;

			/*if (i>=1) Assert(false, std::format("Updated selectable array ({}) element added:{} selectables:{} HIERARCHY:{}", 
				std::to_string(i), element->ToStringBase(), ToStringSelectables(), m_hierarchy.ToStringTree()));
			i++;*/
		});

	m_hierarchy.m_OnElementRemoved.AddListener([this](const GUIElement* element)-> void
		{
			//CreateSelectableArray();
			m_guiTreeUpdated = true;
		});
}

void GUISelectorManager::SelectNewSelectable(SelectableGUI* selectable)
{
	if (selectable == nullptr) return;
	LogWarning(std::format("SELEC NEW SELECTABLE:{}", selectable->ToStringBase()));
	selectable->Select();
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
	static int clickTime = 0;
	//std::string allRect = "";
	//for (auto& selectable : m_selectables)
	//{
	//	if (selectable == nullptr) continue;

	//	allRect += selectable->GetLastFrameRect().ToString();
	//	//selectable->GetLastFrameRect();

	//	//allRect += "HELLO";
	//}

	TryUpdateTree();

	if (m_currentSelected != nullptr && m_selectedThisFrame) m_selectedThisFrame = false;
	const Input::KeyState selectKeyState = m_inputManager.GetInputKey(SELECT_KEY)->GetState().GetState();

	const Vec2 mousePos = m_inputManager.GetMousePosition();
	const Vec2 mouseDelta = mousePos - m_lastFrameMousePos;
	m_lastFrameMousePos = mousePos;

	//Note: since the initial click on a draggable selectable is the only thing that matters, we do not care if it moves outside the area
	//so we can easily just update the current one that is dragged
	if (selectKeyState == Input::KeyState::Down && m_currentDragged != nullptr)
	{
		//LogError(std::format("Updating drag for:{} all selectable:{}", typeid(*m_currentDragged).name(), ToStringSelectableTypes()));
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
			/*LogError(std::format("checking selectable:{} last rect:{} contains pos:{}->{}", selectable->ToStringBase(), 
				selectable->GetLastFrameRect().ToString(), mousePos.ToString(), std::to_string(selectable->GetLastFrameRect().ContainsPos(mousePos))));*/

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
		}
	}
	//clickTime++;
	if (selectKeyState == Input::KeyState::Released && clickTime>2) 
		Assert(false, std::format("After event CLICKED:{} \n\n selectavle tree:{} \n\ngui tree:{}", mousePos.ToString(), ToStringSelectables(), m_hierarchy.ToStringTree()));
	
	//Assert(false, std::format("All rects: {}", allRect));

	

	/*LogError(std::format("Selector has: {} selectables active type: {} (rect: {}) allrect: {}", m_selectables.size(), m_currentSelected != nullptr ?
		ToString(dynamic_cast<InputField*>(m_currentSelected)->GetFieldType()) : "NULL", 
		m_currentSelected!=nullptr? m_currentSelected->GetLastFrameRect().ToString() : "NULL", allRect));*/
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

std::string GUISelectorManager::ToStringSelectables() const
{
	std::vector<std::string> layersList = {};
	std::vector<std::string> selectabeList = {};

	for (const auto& layer : m_selectableLayers)
	{
		layersList.push_back("[Layer, Selectables:");
		for (const auto& selectable : layer.second)
		{
			selectabeList.push_back(selectable->ToStringBase());
		}
		layersList.back() += Utils::ToStringIterable(selectabeList);
		layersList.back().push_back(']');
	}
	return Utils::ToStringIterable(layersList);
}
