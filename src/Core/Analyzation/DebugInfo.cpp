#include "pch.hpp"
#include "Core/Analyzation/DebugInfo.hpp"
#include "StaticGlobals.hpp"
#include "Core/Scene/Scene.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/World/PhysicsBodyComponent.hpp"
#include "ECS/Component/Types/World/PlayerComponent.hpp"
#include "ECS/Component/Types/World/CameraComponent.hpp"
#include "Core/Input/InputManager.hpp"
#include "Editor/EditorStyles.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "ECS/Component/Types/UI/UILayoutComponent.hpp"
#include "ECS/Component/Types/UI/UITransformComponent.hpp"
#include "ECS/Component/Types/UI/UITextComponent.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Engine::Editor::Debug
{
	const float TOP_LEFT_Y = 0.95;
	const float DEBUG_AREA_WIDTH = 0.3;
	const float DEBUG_AREA_HIGHER_PER_PROPERTY = 0.05;
	constexpr float TEXT_SIZE = 11;

	using EntityData = Engine::ECS::EntityData;

	DebugInfo::DebugInfo() :
		m_textGuis(::Utils::ConstructArray<MainUI::UITextComponent*, DEBUG_PROPERTIES_COUNT>()),
		m_containerLayout(nullptr),
		m_nextIndex(0), m_mouseDebugData(std::nullopt), m_isEnabled(false)
	{

	}

	void DebugInfo::CreateUI(MainUI::UIHierarchy& hierarchy)
	{
		auto [debugInfoEntity, debugInfoTransform] = hierarchy.CreateAtRoot(MainUI::TOP_LAYER, "DebugInfoContainer");
		m_containerLayout = &(debugInfoEntity->AddComponent(MainUI::UILayoutComponent(MainUI::LayoutType::Vertical, MainUI::SizingType::ExpandAndShrink)));
		const NormalizedVec2 topLeft = { 0, TOP_LEFT_Y };
		debugInfoTransform->SetLocalBoundsTLBR(topLeft, { DEBUG_AREA_WIDTH, topLeft.m_Y - DEBUG_AREA_HIGHER_PER_PROPERTY * DEBUG_PROPERTIES_COUNT });

		for (size_t i = 0; i < m_textGuis.size(); i++)
		{
			auto [textEntity, textTransoform] = m_containerLayout->CreateLayoutElement("TextDebug");
			m_textGuis[i] = &(textEntity->AddComponent(MainUI::UITextComponent("", Editor::Styles::GetTextStyleSetSize(MainUI::TextAlignment::CenterLeft, TEXT_SIZE))));
			textTransoform->SetFixed(true, false);
		}
	}

	void DebugInfo::SetProperty(const std::string& name, const std::string& value)
	{
		std::string fullStr = std::format("{}: {}", name, value);
		m_textGuis[m_nextIndex]->SetText(fullStr);

		m_nextIndex++;
	}

	void DebugInfo::SetMouseDebugData(const DebugMousePosition& info)
	{
		m_mouseDebugData = info;
	}
	const std::optional<DebugMousePosition>& DebugInfo::GetMouseDebugData() const
	{
		return m_mouseDebugData;
	}

	void DebugInfo::Update(const float& deltaTime, const float& timeStep, Scenes::Scene& activeScene, 
		const Input::InputManager& input, const Camera::CameraComponent& mainCamera)
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
		SetProperty("FPS", std::format("{} fps", std::to_string(1.0f / deltaTime)));
		SetProperty("DeltaTime", std::format("{} s", std::to_string(deltaTime)));
		SetProperty("TimeStep", std::format("{} s", std::to_string(timeStep)));

		EntityData* playerEntity = activeScene.TryGetEntityMutable("player", true);
		if (!Assert(playerEntity != nullptr, "Tried to update properties"
			"for debug info but player could not be in active scene"))
			return;

		SetProperty("KeysDown", ::Utils::ToStringIterable(input.GetAllKeysWithStateAsString(Input::KeyState::Down)));

		const Physics::PhysicsBodyComponent* maybePhysics = playerEntity->TryGetComponent<Physics::PhysicsBodyComponent>();
		const Player::PlayerComponent* maybePlayer = playerEntity->TryGetComponent<Player::PlayerComponent>();
		SetProperty("Input", std::format("{}", maybePlayer->GetFrameInput().ToString()));
		SetProperty("PlayerGPos", std::format("{} m", playerEntity->GetTransformMutable().GetWorldPos().ToString()));
		SetProperty("PlayerVel", std::format("{} m/s", maybePhysics->GetVelocity().ToString(3)));
		SetProperty("PlayerAcc", std::format("{} m/s2", maybePhysics->GetAcceleration().ToString(3)));
		SetProperty("Grounded:", std::format("{}", std::to_string(maybePlayer->GetIsGrounded())));
		SetProperty("GroundDist:", std::format("{} m", std::to_string(maybePlayer->GetVerticalDistanceToGround())));

		ScreenPosition mouseScreenPos = input.GetMousePosition();
		Math::Ray3D mouseWorld = mainCamera.ScreenToWorldPosition(mouseScreenPos);
		SetMouseDebugData(DebugMousePosition{ mouseWorld, ScreenPosition{mouseScreenPos.m_X + 15, mouseScreenPos.m_Y} });
		//LogError(std::format("Finished update loop"));
	}
}
