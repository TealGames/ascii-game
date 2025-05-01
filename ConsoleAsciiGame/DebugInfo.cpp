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

DebugInfo::DebugInfo() : 
	m_text(), m_highlightedIndices(), m_mouseDebugData(std::nullopt), m_isEnabled(false) {}

void DebugInfo::ClearProperties()
{
	m_text.clear();
}
void DebugInfo::AddProperty(const std::string& name, const std::string& value)
{
	std::string fullStr = std::format("{}: {}", name, value);
	m_text.emplace_back(fullStr.c_str());
}

const std::vector<std::string>& DebugInfo::GetText() const
{
	return m_text;
}

bool DebugInfo::TryAddHighlightedIndex(const size_t& index)
{
	if (std::find(m_highlightedIndices.begin(), m_highlightedIndices.end(), index) !=
		m_highlightedIndices.end()) return false;

	m_highlightedIndices.emplace_back(index);
	//TODO: yes this is inneficient but we want to make sure get function does not mutate the data
	std::sort(m_highlightedIndices.begin(), m_highlightedIndices.end());
	return true;
}
void DebugInfo::RemoveHighlightedIndex(const size_t& index)
{
	auto it = std::find(m_highlightedIndices.begin(), m_highlightedIndices.end(), index);
	if (it == m_highlightedIndices.end()) return;

	m_highlightedIndices.erase(it);
}
void DebugInfo::ClearHighlightedIndices()
{
	m_highlightedIndices.clear();
}
const std::vector<std::size_t>& DebugInfo::GetHighlightedIndicesSorted() const
{
	return m_highlightedIndices;
}

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
	ClearProperties();
	if (input.IsKeyPressed(TOGGLE_DEBUG_INFO_KEY))
	{
		m_isEnabled = !m_isEnabled;
	}
	if (!m_isEnabled) return;

	AddProperty("FPS", std::format("{} fps", std::to_string(GetFPS())));
	AddProperty("DeltaTime", std::format("{} s", std::to_string(deltaTime)));
	AddProperty("TimeStep", std::format("{} s", std::to_string(timeStep)));

	const ECS::Entity* playerEntity = activeScene.TryGetEntity("player", true);
	if (!Assert(this, playerEntity != nullptr, std::format("Tried to update properties"
		"for debug info but player could not be in active scene")))
		return;

	AddProperty("KeysDown", Utils::ToStringIterable<std::vector<std::string>,
		std::string>(input.GetAllKeysWithStateAsString(Input::KeyState::Down)));

	const PhysicsBodyData* maybePhysics = playerEntity->TryGetComponent<PhysicsBodyData>();
	const PlayerData* maybePlayer = playerEntity->TryGetComponent<PlayerData>();
	AddProperty("Input", std::format("{}", maybePlayer->GetFrameInput().ToString()));
	AddProperty("PlayerPos", std::format("{} m", playerEntity->m_Transform.GetPos().ToString()));
	AddProperty("PlayerVel", std::format("{} m/s", maybePhysics->GetVelocity().ToString(3, VectorForm::Component)));
	AddProperty("PlayerAcc", std::format("{} m/s2", maybePhysics->GetAcceleration().ToString(3, VectorForm::Component)));
	AddProperty("Grounded:", std::format("{}", std::to_string(maybePlayer->GetIsGrounded())));
	AddProperty("GroundDist:", std::format("{} m", std::to_string(maybePlayer->GetVerticalDistanceToGround())));

	Vector2 mousePos = GetMousePosition();
	ScreenPosition mouseScreenPos = { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y) };
	WorldPosition mouseWorld = Conversions::ScreenToWorldPosition(mainCamera, mouseScreenPos);
	SetMouseDebugData(DebugMousePosition{ mouseWorld, {mouseScreenPos.m_X + 15, mouseScreenPos.m_Y} });
}

bool DebugInfo::TryRender()
{
	Vector2 startPos = { 5, 5 };
	Vector2 currentPos = startPos;
	Vector2 currentSize = {};

	Color defaultColor = DEBUG_TEXT_COLOR;
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
			currentColor = DEBUG_HIGHLIGHTED_TEXT_COLOR;
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