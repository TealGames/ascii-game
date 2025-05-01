#include "pch.hpp"
#include "SpriteEditorGUI.hpp"
#include "Debug.hpp"
#include "GUISelectorManager.hpp"
#include "EntityEditorGUI.hpp"
#include "AssetManager.hpp"
#include "SpriteAsset.hpp"

SpriteEditorGUI::SpriteEditorGUI(GUISelectorManager& guiSelector, const Input::InputManager& inputManager, AssetManagement::AssetManager& assetManager)
	: m_selectorManager(&guiSelector), m_assetManager(&assetManager), m_inputManager(&inputManager), m_saveButton(guiSelector, GUISettings(), "Save")
{
	const GUISettings buttonSettings = GUISettings({ 20, 20 }, EntityEditorGUI::EDITOR_PRIMARY_COLOR,
		TextGUISettings(EntityEditorGUI::EDITOR_TEXT_COLOR, FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()),
		TextAlignment::Center, GUIPadding(), 0.8));

	m_saveButton.SetSettings(buttonSettings);
	m_saveButton.SetClickAction([this](const ButtonGUI& button)-> void
		{
			//TODO: find a way for user to input path and then set it here
			//GetAssetManager().TryCreateNewAsset<SpriteAsset>("");
		});
}

void SpriteEditorGUI::Update(const float deltaTime)
{
	m_saveButton.Update(deltaTime);
}
ScreenPosition SpriteEditorGUI::Render(const RenderInfo& renderInfo)
{
	m_saveButton.Render(RenderInfo(renderInfo.m_TopLeftPos, ScreenPosition(60, 20)));
	return renderInfo.m_RenderSize;
}