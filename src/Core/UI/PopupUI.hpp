#pragma once

namespace Engine::ECS { class EntityData; }
namespace Engine::UI
{
	class UITransformComponent;
	struct PopupUI
	{
		UITransformComponent* m_Container;

		PopupUI();

		void CreatePopup(UITransformComponent& parent);
		virtual void AddPopupElements() = 0;
		virtual ~PopupUI() = default;
	};
}