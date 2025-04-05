#pragma once
#include <string>
#include <vector>
#include <optional>
#include "WorldPosition.hpp"
#include "ScreenPosition.hpp"
#include "IBasicRenderable.hpp"
#include "raylib.h"

class Scene;
namespace Input
{
	class InputManager;
}
class CameraData;

struct DebugMousePosition
{
	WorldPosition m_MouseWorldPos = {};
	ScreenPosition m_MouseTextScreenPos = {};
};

class DebugInfo : public IBasicRenderable
{
private:
	std::vector<std::string> m_text;
	std::vector<std::size_t> m_highlightedIndices;

	std::optional<DebugMousePosition> m_mouseDebugData;

	bool m_isEnabled;
public:
	static constexpr KeyboardKey TOGGLE_DEBUG_INFO_KEY = KEY_TAB;

private:
public:
	DebugInfo();

	void ClearProperties();
	void Update(const float& deltaTime, const float& timeStep, const Scene& activeScene, const Input::InputManager& input, const CameraData& mainCamera);
	bool TryRender() override;

	void AddProperty(const std::string& name, const std::string& value);
	const std::vector<std::string>& GetText() const;

	bool TryAddHighlightedIndex(const size_t& index);
	void RemoveHighlightedIndex(const size_t& index);
	void ClearHighlightedIndices();
	const std::vector<std::size_t>& GetHighlightedIndicesSorted() const;

	const std::optional<DebugMousePosition>& GetMouseDebugData() const;
	void SetMouseDebugData(const DebugMousePosition&);
};

