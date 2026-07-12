#include <queue>
#include <vector>
#include "pch.hpp"
#include "Core/UI/UIHierarchy.hpp"
#include "Utils/Debug.hpp"
#include "Core/Scene/GlobalEntityManager.hpp"
#include "ECS/Component/Types/World/TransformComponent.hpp"

namespace Engine::UI
{
	static constexpr bool DRAW_RENDER_BOUNDS = false;

	UIHierarchy::UIHierarchy(Scenes::GlobalEntityManager& globalEntityManager)
		: m_globalEntityManager(&globalEntityManager), m_uiRoot(nullptr), m_layerRoots({}),
		m_OnElementAdded(), m_OnElementRemoved(), m_uiHierarchyUpdatedThisFrame(false)
	{

	}

	void UIHierarchy::Init()
	{
		//m_rootElements.reserve(TOP_LAYER);
		ECS::EntityData& uiRootEntity = m_globalEntityManager->CreateGlobalEntity(ROOT_UI_ENTITY_NAME, TransformComponent());
		UITransformComponent& rootTransform = uiRootEntity.AddComponent<UITransformComponent>();
		//(std::format("Root transform: {}", rootTransform.ToString()));
		m_uiRoot = &rootTransform;

		if (m_uiRoot == nullptr)
		{
			LogError(std::format("Attmpted to create the ui root element "
				"but its ui transform element was null. Entity:{}", uiRootEntity.ToString()));
		}

		ECS::EntityData::OnChildElementAdded.AddListener([this](ECS::EntityData* parentEntity, ECS::EntityData* childEntity, size_t childIndex)-> void
			{
				if (!childEntity->HasComponent<UITransformComponent>())
					return;

				ECS::EntityData* currentParent = parentEntity;
				while (currentParent->GetParent() != nullptr)
				{
					currentParent = currentParent->GetParentMutable();
				}
				//LogWarning(std::format("Added highest parent:{}", currentParent->ToString()));
				if (currentParent->GetId() != m_uiRoot->GetEntityID()) return;

				m_OnElementAdded.Invoke(currentParent->TryGetComponentMutable<UITransformComponent>());
				m_uiHierarchyUpdatedThisFrame = true;
				//LogError("Hierarchy updated");
			});
		//m_rootElements.reserve(MAX_LAYERS);
	}
	void UIHierarchy::Update()
	{
		m_uiHierarchyUpdatedThisFrame = false;
	}
	bool UIHierarchy::WasUIHierarchyUpdatedThisFrame() const
	{
		return m_uiHierarchyUpdatedThisFrame;
	}

	bool UIHierarchy::IsValidLayer(const UILayer layer, const bool logError) const
	{
		const bool isValid = layer <= TOP_LAYER;
		if (logError && !isValid)
		{
			LogError(std::format("Attempted to use a gui layer:{} "
				"but the underlying layer value is invalid", std::to_string(layer)));
		}
		return isValid;
	}

	UITransformComponent* UIHierarchy::CreateNewLayer(const UILayer layer)
	{
		if (!IsValidLayer(layer, true)) return nullptr;
		if (m_layerRoots[layer] != nullptr)
		{
			LogError(std::format("Attempted to create a new layer:{} in gui hierarchy "
				"but that layer has already been created", std::to_string(layer)));
			return nullptr;
		}

		ECS::EntityData& layerRoot = m_uiRoot->GetEntityMutable().CreateChild(std::format("Layer{}", std::to_string(layer)), TransformComponent());
		UITransformComponent& rootElement = layerRoot.AddComponent<UITransformComponent>();
		m_layerRoots[layer] = &rootElement;

		return &rootElement;
	}
	bool UIHierarchy::IsLayerRootID(const ECS::EntityID id) const
	{
		//TODO: we could speed up this search if we reserve each layer with its own range

		//NOTE: we could go through the root of each layer OR we can just look at cached
		//root elements, but we do root elements since they are faster to access
		if (m_uiRoot->GetEntity().GetChildCount() == 0) return false;

		for (const auto& rootElement : m_layerRoots)
		{
			if (rootElement == nullptr) continue;
			if (rootElement->GetEntity().GetId() == id)
				return true;
		}
		return false;
	}

	UITransformComponent* UIHierarchy::FindMutable(const ECS::EntityID id)
	{
		//TODO: there could be a speed up by having each layer be reserved some amounts of ids so we dont have to search all layers
		UITransformComponent* foundElement = nullptr;
		for (size_t i = 0; i < m_layerRoots.size(); i++)
		{
			if (m_layerRoots[i] == nullptr) continue;

			foundElement = m_layerRoots[i]->GetEntityMutable().FindComponentRecursiveMutable<UITransformComponent>(id);
			if (foundElement != nullptr) return foundElement;
		}
		return nullptr;
	}
	UITransformComponent* UIHierarchy::FindParentMutable(const ECS::EntityID id, size_t* foundChildIndex)
	{
		UITransformComponent* foundEntity = nullptr;
		for (size_t i = 0; i < m_layerRoots.size(); i++)
		{
			if (m_layerRoots[i] == nullptr) continue;

			foundEntity = m_layerRoots[i]->GetEntityMutable().FindParentComponentRecursiveMutable<UITransformComponent>(id, foundChildIndex);
			if (foundEntity != nullptr) return foundEntity;
		}
		return nullptr;
	}

	std::vector<UITransformComponent*> UIHierarchy::GetLayerRootsMutable(const bool topLayerFirst)
	{
		if (m_layerRoots.empty()) return {};
		std::vector<UITransformComponent*> layers = {};


		if (topLayerFirst)
		{
			for (int i = m_layerRoots.size() - 1; i >= 0; i--)
				layers.push_back(m_layerRoots[i]);
		}
		else
		{
			for (auto& layer : m_layerRoots)
			{
				if (layer == nullptr) continue;
				layers.push_back(layer);
			}
		}
		return layers;
	}

	const UITransformComponent* UIHierarchy::GetRootElement() const { return m_uiRoot; }
	const UITransformComponent* UIHierarchy::GetLayerRootElement(const UILayer layer) const
	{
		if (!IsValidLayer(layer, true)) return nullptr;
		return m_layerRoots[layer];
	}

	std::tuple<ECS::EntityData*, UITransformComponent*> UIHierarchy::CreateAtRoot(const UILayer layer, const std::string& name)
	{
		if (!IsValidLayer(layer, true)) return std::make_tuple<ECS::EntityData*, UITransformComponent*>(nullptr, nullptr);
		if (m_layerRoots[layer] == nullptr) CreateNewLayer(layer);
		return m_layerRoots[layer]->GetEntityMutable().CreateChildUI(name);
	}

	void UIHierarchy::AddToRoot(const UILayer layer, UITransformComponent* element)
	{
		if (!IsValidLayer(layer, true)) return;
		//Note: we could use the existing functions to accomplish this, but we can make assumptions
		//can speed up some of the process here
		if (element == nullptr)
		{
			LogError(std::format("Tried to add element to root of gui hierarchy but element is NULL"));
			return;
		}

		if (m_layerRoots[layer] == nullptr) CreateNewLayer(layer);

		m_layerRoots[layer]->GetEntityMutable().PushChild(element->GetEntityMutable());
		//m_OnElementAdded.Invoke(element);
	}
	UITransformComponent* UIHierarchy::RemoveFromRoot(const UILayer layer, const ECS::EntityID id)
	{
		if (IsLayerRootID(id))
		{
			LogError(std::format("Tried to remove element with id:'{}' from hierarchy, "
				"but it is a layer root element and those cannot be removed", ECS::ToString(id)));
			return nullptr;
		}
		if (m_layerRoots[layer] == nullptr)
		{
			LogError(std::format("Attempted to remove element from layer:{} "
				"root with id:{} but that layer has no root", std::to_string(layer), ECS::ToString(id)));
			return nullptr;
		}

		return m_layerRoots[layer]->GetEntityMutable().TryPopChildAs<UITransformComponent>(id);
	}

	void UIHierarchy::ClearLayer(const UILayer layer)
	{
		if (!IsValidLayer(layer, true)) return;
		if (m_layerRoots[layer] == nullptr) return;

		m_layerRoots[layer]->GetEntityMutable().PopAllChildren();
	}

	std::optional<UIRect> TryCalculateRenderRectHelper(const UITransformComponent& currentTransform, const UIRect& parentRect, const ECS::EntityID targetId)
	{
		UIRect thisRect = currentTransform.CalculateWorldRect(parentRect);
		if (currentTransform.GetEntityID() == targetId) return thisRect;

		std::optional<UIRect> childResult = std::nullopt;
		for (const auto& child : currentTransform.GetEntity().GetChildrenOfType<UITransformComponent>())
		{
			childResult = TryCalculateRenderRectHelper(currentTransform, currentTransform.CalculateChildParentRect(thisRect), targetId);
			if (childResult != std::nullopt) return childResult;
		}
		return std::nullopt;
	}
	std::optional<UIRect> UIHierarchy::TryCalculateRenderRect(const UITransformComponent& element)
	{
		std::optional<UIRect> layerResult = std::nullopt;
		for (const auto& layer : m_layerRoots)
		{
			if (layer == nullptr) continue;
			layerResult = TryCalculateRenderRectHelper(*layer, GetRootRect(), element.GetEntityID());
			if (layerResult != std::nullopt) return layerResult;
		}
		return std::nullopt;
	}

	void UIHierarchy::LayerTraversal(const std::function<void(UILayer, UITransformComponent&)>& action)
	{
		for (size_t i = 0; i < m_layerRoots.size(); i++)
		{
			if (m_layerRoots[i] == nullptr) continue;
			action(static_cast<UILayer>(i), *m_layerRoots[i]);
		}
	}

	UIRect UIHierarchy::GetRootRect() const
	{
		return UIRect(UI_RECT_BOTTOM_LEFT, UI_RECT_TOP_RIGHT - UI_RECT_BOTTOM_LEFT);
	}

	std::string UIHierarchy::ToStringElementHelper(std::string startNewLine, const UITransformComponent& element) const
	{
		std::string result = "";
		result += std::format("\n{}-> {}", startNewLine, std::format("{}({})", element.GetEntity().ToString(), element.ToString()));
		//result += std::format("\n{}-> {}", startNewLine, std::format("{} ({})", element.GetEntity().m_Name, element.ToString()));

		if (element.GetEntity().GetChildCount() == 0) return result;

		startNewLine += "    ";
		for (const auto& child : element.GetEntity().GetChildrenOfType<UITransformComponent>())
		{
			if (child == nullptr) continue;
			result += ToStringElementHelper(startNewLine, *child);
		}

		return result;
	}

	std::string UIHierarchy::ToStringTree() const
	{
		std::string result = std::format("\n[Layers:{}]", std::to_string(m_layerRoots.size()));

		for (size_t i = 0; i < m_layerRoots.size(); i++)
		{
			result += std::format("\n\nLayer{}{}:{}", std::to_string(i),
				i == BOTTOM_LAYER ? "(BOTTOM)" : "",
				m_layerRoots[i] != nullptr ? ToStringElementHelper("", *m_layerRoots[i]) : "NULL");
		}
		return result;
	}
}
