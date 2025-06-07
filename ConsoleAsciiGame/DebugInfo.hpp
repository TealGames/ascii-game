#pragma once
#include <string>
#include <array>
#include <optional>
#include "WorldPosition.hpp"
#include "ScreenPosition.hpp"
#include "UITextComponent.hpp"
#include "UIContainer.hpp"
#include "UILayout.hpp"
#include "raylib.h"

class Scene;
namespace Input
{
	class InputManager;
}
class CameraData;
class GUIHierarchy;

struct DebugMousePosition
{
	WorldPosition m_MouseWorldPos = {};
	ScreenPosition m_MouseTextScreenPos = {};
};

constexpr int DEBUG_PROPERTIES_COUNT = 10;
class DebugInfo //: public IBasicRenderable
{
private:
	UIContainer m_container;
	UILayout m_layout;
	std::array<UITextComponent, DEBUG_PROPERTIES_COUNT> m_textGuis;
	size_t m_nextIndex;
	//std::vector<std::size_t> m_highlightedIndices;

	std::optional<DebugMousePosition> m_mouseDebugData;

	bool m_isEnabled;
public:
	static constexpr KeyboardKey TOGGLE_DEBUG_INFO_KEY = KEY_TAB;

private:
public:
	DebugInfo(GUIHierarchy& hierarchy);

	//void ClearProperties();
	void Update(const float& deltaTime, const float& timeStep, const Scene& activeScene, const Input::InputManager& input, const CameraData& mainCamera);
	//bool TryRender() override;

	void SetProperty(const std::string& name, const std::string& value);
	//const std::vector<std::string>& GetText() const;

	/*bool TryAddHighlightedIndex(const size_t& index);
	void RemoveHighlightedIndex(const size_t& index);
	void ClearHighlightedIndices();
	const std::vector<std::size_t>& GetHighlightedIndicesSorted() const;*/

	const std::optional<DebugMousePosition>& GetMouseDebugData() const;
	void SetMouseDebugData(const DebugMousePosition&);
};

