#pragma once
#include <string>
#include <array>
#include <optional>
#include "Core/Primitives/WorldPosition.hpp"
#include "Core/Primitives/ScreenPosition.hpp"
#include "Math/Ray.hpp"
#include "Core/Input/InputKey.hpp"
//#include "raylib.h"

namespace Engine::Scenes { class Scene; }
namespace Engine::Input { class InputManager; }
namespace Engine::Camera { class CameraComponent; }
namespace Engine::UI 
{
	class UIHierarchy;
	class UITransformComponent;
	class UITextComponent;
	class UILayoutComponent;
}
namespace Engine::Editor::Debug
{
	struct DebugMousePosition
	{
		Math::Ray3D m_MouseClickedRay = {};
		ScreenPosition m_MouseTextScreenPos = {};
	};

	namespace MainUI = Engine::UI;

	constexpr int DEBUG_PROPERTIES_COUNT = 10;
	class DebugInfo //: public IBasicRenderable
	{
	private:
		MainUI::UILayoutComponent* m_containerLayout;
		std::array<MainUI::UITextComponent*, DEBUG_PROPERTIES_COUNT> m_textGuis;
		size_t m_nextIndex;
		//std::vector<std::size_t> m_highlightedIndices;

		std::optional<DebugMousePosition> m_mouseDebugData;

		bool m_isEnabled;
	public:
		static constexpr Input::KeyCode TOGGLE_DEBUG_INFO_KEY = Input::KeyCode::Tab;

	private:
	public:
		DebugInfo();

		void CreateUI(MainUI::UIHierarchy& hierarchy);

		void Update(const float& deltaTime, const float& timeStep, Scenes::Scene& activeScene, const Input::InputManager& input, 
			const Camera::CameraComponent& mainCamera);

		void SetProperty(const std::string& name, const std::string& value);

		const std::optional<DebugMousePosition>& GetMouseDebugData() const;
		void SetMouseDebugData(const DebugMousePosition&);
	};
}

