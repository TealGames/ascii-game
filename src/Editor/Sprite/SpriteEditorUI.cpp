#include "pch.hpp"
#include "Editor/Sprite/SpriteEditorUI.hpp"
#include "Core/Analyzation/Debug.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "Editor/Entity/EntityEditorUI.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Asset/SpriteAsset.hpp"

SpriteEditorUI::SpriteEditorUI(UIHierarchy& hierarchy, const Input::InputManager& inputManager, AssetManagement::AssetManager& assetManager)
	: m_assetManager(&assetManager), m_inputManager(&inputManager) //,m_saveButton(GUIStyle(), "Save")
{
	/*const GUIStyle buttonSettings = GUIStyle(EntityEditorGUI::EDITOR_PRIMARY_COLOR,
		TextGUIStyle(EntityEditorGUI::EDITOR_TEXT_COLOR, FontProperties(0, EntityEditorGUI::EDITOR_CHAR_SPACING.m_X, GetGlobalFont()),
		TextAlignment::Center, GUIPadding(), 0.8));*/

	//m_saveButton.SetSize({ 0.1, 0.05 });
	//m_saveButton.SetSettings(buttonSettings);
	//m_saveButton.SetClickAction([this](const ButtonGUI& button)-> void
	//	{
	//		//TODO: find a way for user to input path and then set it here
	//		//GetAssetManager().TryCreateNewAsset<SpriteAsset>("");
	//	});

	//m_guiContainer.PushChild(&m_saveButton);
	//hierarchy.AddToRoot(DEFAULT_LAYER, &m_guiContainer);
}

void SpriteEditorUI::Update(const float deltaTime)
{
	//m_saveButton.Update(deltaTime);
}
/*
ScreenPosition SpriteEditorGUI::Render(const RenderInfo& renderInfo)
{
	m_saveButton.Render(RenderInfo(renderInfo.m_TopLeftPos, ScreenPosition(60, 20)));
	return renderInfo.m_RenderSize;
}
*/