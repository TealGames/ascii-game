#include "pch.hpp"
#include "Core/Analyzation/DebugInfo.hpp"
#include "StaticGlobals.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/World/PhysicsBodyData.hpp"
#include "ECS/Component/Types/World/PlayerData.hpp"
#include "ECS/Component/Types/World/CameraData.hpp"
#include "Core/PositionConversions.hpp"
#include "Core/Input/InputManager.hpp"
#include "Utils/RaylibUtils.hpp"
#include "Editor/EditorStyles.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "ECS/Component/Types/UI/UILayout.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"

const float TOP_LEFT_Y = 0.95;
const float DEBUG_AREA_WIDTH = 0.3;
const float DEBUG_AREA_HIGHER_PER_PROPERTY = 0.05;
constexpr float TEXT_SIZE = 11;

DebugInfo::DebugInfo() :
	m_textGuis(Utils::ConstructArray<UITextComponent*, DEBUG_PROPERTIES_COUNT>()),
	m_containerLayout(nullptr),
	m_nextIndex(0), m_mouseDebugData(std::nullopt), m_isEnabled(false)
{
	
}

void DebugInfo::CreateUI(UIHierarchy& hierarchy)
{
	auto [debugInfoEntity, debugInfoTransform] = hierarchy.CreateAtRoot(TOP_LAYER, "DebugInfoContainer");
	m_containerLayout = &(debugInfoEntity->AddComponent(UILayout(LayoutType::Vertical, SizingType::ExpandAndShrink)));
	const NormalizedPosition topLeft = { 0, TOP_LEFT_Y };
	debugInfoTransform->SetBounds(topLeft, { DEBUG_AREA_WIDTH, topLeft.m_Y - DEBUG_AREA_HIGHER_PER_PROPERTY * DEBUG_PROPERTIES_COUNT });

	for (size_t i = 0; i < m_textGuis.size(); i++)
	{
		auto [textEntity, textTransoform] = m_containerLayout->CreateLayoutElement("TextDebug");
		m_textGuis[i] = &(textEntity->AddComponent(UITextComponent("", EditorStyles::GetTextStyleSetSize(TextAlignment::CenterLeft, TEXT_SIZE))));
		textTransoform->SetFixed(true, false);
	}
}

//void DebugInfo::ClearProperties()
//{
//	m_container.PopAllChildren();
//	//m_textGuis.clear();
//}
void DebugInfo::SetProperty(const std::string& name, const std::string& value)
{
	std::string fullStr = std::format("{}: {}", name, value);
	m_textGuis[m_nextIndex]->SetText(fullStr);

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
	bool isActive = m_isEnabled;
	if (input.IsKeyPressed(TOGGLE_DEBUG_INFO_KEY))
	{
		m_isEnabled = !m_isEnabled;
	}
	m_containerLayout->GetEntityMutable().TrySetEntityActive(m_isEnabled);
	//Log(std::format("Debug info active:{}", m_containerLayout->GetEntityMutable().IsEntityActive()));
	if (!m_isEnabled) return;

	m_nextIndex = 0;
	SetProperty("FPS", std::format("{} fps", std::to_string(GetFPS())));
	SetProperty("DeltaTime", std::format("{} s", std::to_string(deltaTime)));
	SetProperty("TimeStep", std::format("{} s", std::to_string(timeStep)));

	const EntityData* playerEntity = activeScene.TryGetEntity("player", true);
	if (!Assert(playerEntity != nullptr, std::format("Tried to update properties"
		"for debug info but player could not be in active scene")))
		return;

	SetProperty("KeysDown", Utils::ToStringIterable<std::vector<std::string>,
		std::string>(input.GetAllKeysWithStateAsString(Input::KeyState::Down)));

	const PhysicsBodyData* maybePhysics = playerEntity->TryGetComponent<PhysicsBodyData>();
	const PlayerData* maybePlayer = playerEntity->TryGetComponent<PlayerData>();
	SetProperty("Input", std::format("{}", maybePlayer->GetFrameInput().ToString()));
	SetProperty("PlayerGPos", std::format("{} m", playerEntity->GetTransform().GetGlobalPos().ToString()));
	SetProperty("PlayerVel", std::format("{} m/s", maybePhysics->GetVelocity().ToString(3, VectorForm::Component)));
	SetProperty("PlayerAcc", std::format("{} m/s2", maybePhysics->GetAcceleration().ToString(3, VectorForm::Component)));
	SetProperty("Grounded:", std::format("{}", std::to_string(maybePlayer->GetIsGrounded())));
	SetProperty("GroundDist:", std::format("{} m", std::to_string(maybePlayer->GetVerticalDistanceToGround())));

	ScreenPosition mouseScreenPos = input.GetMousePosition();
	WorldPosition mouseWorld = Conversions::ScreenToWorldPosition(mainCamera, mouseScreenPos);
	SetMouseDebugData(DebugMousePosition{ mouseWorld, ScreenPosition{mouseScreenPos.m_X + 15, mouseScreenPos.m_Y} });
	//LogError(std::format("Finished update loop"));
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