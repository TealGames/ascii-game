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

DebugInfo::DebugInfo() : 
	m_text(), m_highlightedIndices(), m_mouseDebugData(std::nullopt) {}

void DebugInfo::ClearProperties()
{
	m_text.clear();
}
void DebugInfo::AddProperty(const std::string& name, const std::string& value)
{
	std::string fullStr = std::format("{}: {}", name, value);
	m_text.push_back(fullStr.c_str());
}

const std::vector<std::string>& DebugInfo::GetText() const
{
	return m_text;
}

bool DebugInfo::TryAddHighlightedIndex(const size_t& index)
{
	if (std::find(m_highlightedIndices.begin(), m_highlightedIndices.end(), index) !=
		m_highlightedIndices.end()) return false;

	m_highlightedIndices.push_back(index);
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

void DebugInfo::UpdateProperties(const float& deltaTime, const float& timeStep, Scene& activeScene, Input::InputManager& input, const CameraData& mainCamera)
{
	ClearProperties();

	AddProperty("FPS", std::format("{} fps", std::to_string(GetFPS())));
	AddProperty("DeltaTime", std::format("{} s", std::to_string(deltaTime)));
	AddProperty("TimeStep", std::format("{} s", std::to_string(timeStep)));

	ECS::Entity* playerEntity = activeScene.TryGetEntityMutable("player", true);
	if (!Assert(this, playerEntity != nullptr, std::format("Tried to update properties"
		"for debug info but player could not be in active scene")))
		return;

	AddProperty("KeysDown", Utils::ToStringIterable<std::vector<std::string>,
		std::string>(input.GetAllKeysWithStateAsString(Input::KeyState::Down)));

	const PhysicsBodyData* maybePhysics = playerEntity->TryGetComponent<PhysicsBodyData>();
	const PlayerData* maybePlayer = playerEntity->TryGetComponent<PlayerData>();
	AddProperty("Input", std::format("{}", maybePlayer->GetFrameInput().ToString()));
	AddProperty("PlayerPos", std::format("{} m", playerEntity->m_Transform.m_Pos.ToString()));
	AddProperty("PlayerVel", std::format("{} m/s", maybePhysics->GetVelocity().ToString(3, VectorForm::Component)));
	AddProperty("PlayerAcc", std::format("{} m/s2", maybePhysics->GetAcceleration().ToString(3, VectorForm::Component)));
	AddProperty("Grounded:", std::format("{}", std::to_string(maybePlayer->GetIsGrounded())));
	AddProperty("GroundDist:", std::format("{} m", std::to_string(maybePlayer->GetVerticalDistanceToGround())));

	Vector2 mousePos = GetMousePosition();
	ScreenPosition mouseScreenPos = { static_cast<int>(mousePos.x), static_cast<int>(mousePos.y) };
	WorldPosition mouseWorld = Conversions::ScreenToWorldPosition(mainCamera, mouseScreenPos);
	SetMouseDebugData(DebugMousePosition{ mouseWorld, {mouseScreenPos.m_X + 15, mouseScreenPos.m_Y} });
}