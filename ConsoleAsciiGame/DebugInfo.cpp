#include "pch.hpp"
#include "DebugInfo.hpp"
#include "Globals.hpp"
#include "Scene.hpp"
#include "Entity.hpp"
#include "PhysicsBodyData.hpp"
#include "PlayerData.hpp"
#include "CameraData.hpp"
#include "PositionConversions.hpp"
#include "InputManager.hpp"
#include "RaylibUtils.hpp"
#include "EditorStyles.hpp"
#include "GUIHierarchy.hpp"

const float TOP_LEFT_Y = 0.95;
const float DEBUG_AREA_WIDTH = 0.3;
const float DEBUG_AREA_HIGHER_PER_PROPERTY = 0.02;
constexpr float TEXT_SIZE = 11;

DebugInfo::DebugInfo(GUIHierarchy& hierarchy) :
	m_textGuis(Utils::ConstructArray<TextGUI, DEBUG_PROPERTIES_COUNT>("", EditorStyles::GetTextStyleSetSize(TextAlignment::CenterLeft, TEXT_SIZE))),
	m_layout(LayoutType::Vertical, SizingType::ExpandAndShrink),
	m_nextIndex(0), m_mouseDebugData(std::nullopt), m_isEnabled(false)
{
	for (auto& text : m_textGuis)
	{
		m_layout.AddLayoutElement(&text);
		text.SetFixed(true, false);
	}
	const NormalizedPosition topLeft = { 0, TOP_LEFT_Y };
	m_container.SetBounds(topLeft, { DEBUG_AREA_WIDTH, topLeft.m_Y - DEBUG_AREA_HIGHER_PER_PROPERTY * DEBUG_PROPERTIES_COUNT });
	hierarchy.AddToRoot(TOP_LAYER, &m_container);
}

//void DebugInfo::ClearProperties()
//{
//	m_container.PopAllChildren();
//	//m_textGuis.clear();
//}
void DebugInfo::SetProperty(const std::string& name, const std::string& value)
{
	std::string fullStr = std::format("{}: {}", name, value);
	m_textGuis[m_nextIndex].SetText(fullStr);

	m_nextIndex++;
}

//const std::vector<std::string>& DebugInfo::GetText() const
//{
//	return m_text;
//}

//bool DebugInfo::TryAddHighlightedIndex(const size_t& index)
//{
//	if (std::find(m_highlightedIndices.begin(), m_highlightedIndices.end(), index) !=
//		m_highlightedIndices.end()) return false;
//
//	m_highlightedIndices.emplace_back(index);
//	//TODO: yes this is inneficient but we want to make sure get function does not mutate the data
//	std::sort(m_highlightedIndices.begin(), m_highlightedIndices.end());
//	return true;
//}
//void DebugInfo::RemoveHighlightedIndex(const size_t& index)
//{
//	auto it = std::find(m_highlightedIndices.begin(), m_highlightedIndices.end(), index);
//	if (it == m_highlightedIndices.end()) return;
//
//	m_highlightedIndices.erase(it);
//}
//void DebugInfo::ClearHighlightedIndices()
//{
//	m_highlightedIndices.clear();
//}
//const std::vector<std::size_t>& DebugInfo::GetHighlightedIndicesSorted() const
//{
//	return m_highlightedIndices;
//}

void DebugInfo::SetMouseDebugData(const DebugMousePosition& info)
{
	m_mouseDebugData = info;
}
const std::optional<DebugMousePosition>& DebugInfo::GetMouseDebugData() const
{
	return m_mouseDebugData;
}

void DebugInfo::Update(const float& deltaTime, const float& timeStep, const Scene& activeScene, const Input::InputManager& input, const CameraData& mainCamera)
{
	if (input.IsKeyPressed(TOGGLE_DEBUG_INFO_KEY))
	{
		m_isEnabled = !m_isEnabled;
		if (m_isEnabled) m_container.PushChild(&m_layout);
		else m_container.TryPopChildAt(0);
	}
	if (!m_isEnabled) return;

	m_nextIndex = 0;
	LogError(std::format("update startg for debug"));
	SetProperty("FPS", std::format("{} fps", std::to_string(GetFPS())));
	SetProperty("DeltaTime", std::format("{} s", std::to_string(deltaTime)));
	SetProperty("TimeStep", std::format("{} s", std::to_string(timeStep)));

	const ECS::Entity* playerEntity = activeScene.TryGetEntity("player", true);
	if (!Assert(this, playerEntity != nullptr, std::format("Tried to update properties"
		"for debug info but player could not be in active scene")))
		return;

	SetProperty("KeysDown", Utils::ToStringIterable<std::vector<std::string>,
		std::string>(input.GetAllKeysWithStateAsString(Input::KeyState::Down)));

	const PhysicsBodyData* maybePhysics = playerEntity->TryGetComponent<PhysicsBodyData>();
	const PlayerData* maybePlayer = playerEntity->TryGetComponent<PlayerData>();
	SetProperty("Input", std::format("{}", maybePlayer->GetFrameInput().ToString()));
	SetProperty("PlayerPos", std::format("{} m", playerEntity->m_Transform.GetPos().ToString()));
	SetProperty("PlayerVel", std::format("{} m/s", maybePhysics->GetVelocity().ToString(3, VectorForm::Component)));
	SetProperty("PlayerAcc", std::format("{} m/s2", maybePhysics->GetAcceleration().ToString(3, VectorForm::Component)));
	SetProperty("Grounded:", std::format("{}", std::to_string(maybePlayer->GetIsGrounded())));
	SetProperty("GroundDist:", std::format("{} m", std::to_string(maybePlayer->GetVerticalDistanceToGround())));

	Vector2 mousePos = GetMousePosition();
	ScreenPosition mouseScreenPos = { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y) };
	WorldPosition mouseWorld = Conversions::ScreenToWorldPosition(mainCamera, mouseScreenPos);
	SetMouseDebugData(DebugMousePosition{ mouseWorld, {mouseScreenPos.m_X + 15, mouseScreenPos.m_Y} });
	LogError(std::format("Finished update loop"));
}

/*
bool DebugInfo::TryRender()
{
	Vector2 startPos = { 5, 5 };
	Vector2 currentPos = startPos;
	Vector2 currentSize = {};

	Color defaultColor = EditorStyles::DEBUG_TEXT_COLOR;
	Color currentColor = defaultColor;

	const auto& highlightedIndices = GetHighlightedIndicesSorted();
	if (!highlightedIndices.empty()) defaultColor.a = 100;

	size_t currentIndex = 0;
	size_t currentCollectionIndex = 0;
	size_t nextHighlightedIndex = !highlightedIndices.empty() ? highlightedIndices[currentCollectionIndex] : -1;

	for (const auto& text : GetText())
	{
		if (!highlightedIndices.empty() && currentIndex == nextHighlightedIndex)
		{
			currentColor = EditorStyles::DEBUG_HIGHLIGHTED_TEXT_COLOR;
			currentCollectionIndex++;
			if (currentCollectionIndex < highlightedIndices.size())
			{
				nextHighlightedIndex = highlightedIndices[currentCollectionIndex];
			}
		}
		else currentColor = defaultColor;

		currentSize = MeasureTextEx(GetGlobalFont(), text.c_str(), DEBUG_INFO_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X);
		DrawTextEx(GetGlobalFont(), text.c_str(), currentPos, DEBUG_INFO_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X, currentColor);

		currentPos.y += currentSize.y + DEBUG_INFO_CHAR_SPACING.m_Y;
		currentIndex++;
	}

	auto maybeMouseData = GetMouseDebugData();
	if (maybeMouseData.has_value())
	{
		std::string textAsStr = std::format("{}", maybeMouseData.value().m_MouseWorldPos.ToString(2));
		const char* text = textAsStr.c_str();
		DrawTextEx(GetGlobalFont(), text, RaylibUtils::ToRaylibVector(maybeMouseData.value().m_MouseTextScreenPos),
			DEBUG_INFO_FONT_SIZE, DEBUG_INFO_CHAR_SPACING.m_X, WHITE);
	}
	return true;
}
*/