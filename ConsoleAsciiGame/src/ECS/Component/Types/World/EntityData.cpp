#include "pch.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Entity/EntityRegistry.hpp"
#include "ECS/Component/Types/UI/UITransformData.hpp"

//TODO: add a reserved entity name ssytem and not allowing those names to be used
const char* EntityData::GLOBAL_SCENE_NAME = "global";

EntityData::EntityData(ECS::EntityRegistry* registry, const ECS::EntityID id,
	const std::string& name, const std::string& sceneName, ECS::EntityID parentId) :
	m_registry(registry), m_id(id), m_Name(name), m_SceneName(sceneName), m_parentId(parentId), m_childrenIds(), 
	m_components(), m_IsSerializable(true), m_OnFarthestChildElementAttached()
{

}

EntityData::EntityData() : EntityData(nullptr, ECS::INVALID_ID, "NULL", "NULL", ECS::INVALID_ID) {}
EntityData::EntityData(ECS::EntityRegistry& registry, const ECS::EntityID id,
	const std::string& name, const std::string& sceneName, EntityData* parent)
	: EntityData(&registry, id, name, sceneName, parent!=nullptr? parent->m_id : ECS::INVALID_ID) {}


ECS::EntityID EntityData::GetId() const { return m_id; }
std::string EntityData::ToStringId() const { return ToString(GetId()); }
bool EntityData::IsGlobal() const { return m_SceneName == GLOBAL_SCENE_NAME; }
ECS::EntityRegistry& EntityData::GetRegistryMutable()
{
	if (m_registry == nullptr)
	{
		Assert(false, std::format("Attempted to get entity registry MUTABLE for entity:{} but it was NULL", ToString()));
		throw std::invalid_argument("Invalid registry state");
	}
	return *m_registry;
}
const ECS::EntityRegistry& EntityData::GetRegistry() const
{
	if (m_registry == nullptr)
	{
		Assert(false, std::format("Attempted to get entity registry IMMUTABLE for entity:{} but it was NULL", ToString()));
		throw std::invalid_argument("Invalid registry state");
	}
	return *m_registry;
}

//const ECS::Entity EntityData::GetParent() const
//{
//	return ECS::Entity(GetEntitySafeMutable().GetRegistryMutable(), m_parentId);
//}

TransformData& EntityData::GetTransformMutable()
{
	TransformData* transform = TryGetComponentMutable<TransformData>();
	if (transform == nullptr)
	{
		Assert(false, std::format("Attempted to get transform MUTABLE from entity:{} but failed", ToString()));
		throw std::invalid_argument("Invalid transform");
	}
	return *transform;
}
const TransformData& EntityData::GetTransform() const
{
	const TransformData* transform = TryGetComponent<TransformData>();
	if (transform == nullptr)
	{
		Assert(false, std::format("Attempted to get transform IMMUTABLE from entity:{} but failed", ToString()));
		throw std::invalid_argument("Invalid transform");
	}
	return *transform;
}
bool EntityData::IsEntityActive() const { return m_isActive; }

bool EntityData::TryActivateEntity()
{
	if (m_isActive) return true;
	//If parent is not enabled, we can not enable this object
	if (!m_registry->TryGetEntity(m_parentId)->m_isActive)
		return false;

	m_isActive = true;
	return true;
}
void EntityData::DeactivateEntity()
{
	if (!m_isActive) return;

	m_isActive = false;
	if (m_childrenIds.empty()) return;

	for (auto& childId : m_childrenIds)
	{
		m_registry->TryGetEntityMutable(childId)->DeactivateEntity();
	}
	return;
}
bool EntityData::TrySetEntityActive(const bool active)
{
	if (active) return TryActivateEntity();
	else
	{
		DeactivateEntity();
		return true;
	}
}

bool EntityData::HasComponent(const std::string& targetComponentName) const
{
	if (targetComponentName.empty()) return false;

	for (const auto& component : m_components)
	{
		if (TryGetComponentName(component) == targetComponentName)
			return true;
	}
	return false;
}
bool EntityData::HasComponent(const std::type_info& typeInfo) const
{
	if (m_components.empty()) return false;
	for (const auto& component : m_components)
	{
		if (component == nullptr) continue;
		if (GetComponentType(component) == typeInfo)
			return true;
	}
	return false;
}
const Component* EntityData::TryGetComponentAtIndex(const size_t& index) const
{
	if (index < 0 || index >= m_components.size())
		return nullptr;

	return m_components[index];
}
Component* EntityData::TryGetComponentAtIndexMutable(const size_t& index)
{
	if (index < 0 || index >= m_components.size())
		return nullptr;

	return m_components[index];
}
const Component* EntityData::TryGetComponentWithName(const std::string& name) const
{
	for (const auto& component : m_components)
	{
		if (component == nullptr) continue;
		if (Utils::FormatTypeName(typeid(*component).name()) == name)
			return component;
	}
	return nullptr;
}
Component* EntityData::TryGetComponentWithNameMutable(const std::string& name)
{
	for (auto& component : m_components)
	{
		if (component == nullptr) continue;
		if (Utils::FormatTypeName(typeid(*component).name()) == name)
			return component;
	}
	return nullptr;
}
size_t EntityData::TryGetIndexOfComponent(const Component* targetComponent) const
{
	size_t index = 0;
	for (const auto& component : m_components)
	{
		if (component == nullptr) continue;
		if (component == targetComponent)
			return index;

		index++;
	}
	return -1;
}
size_t EntityData::TryGetIndexOfComponent(const std::string& componentName) const
{
	size_t index = 0;
	for (const auto& component : m_components)
	{
		if (component == nullptr) continue;
		if (Utils::FormatTypeName(typeid(*component).name()) == componentName)
			return index;

		index++;
	}
	return -1;
}
std::string EntityData::TryGetComponentName(const Component* component) const
{
	return Utils::FormatTypeName(typeid(*component).name());
}
const std::type_info& EntityData::GetComponentType(const Component* component) const
{
	return typeid(*component);
}
std::vector<Component*>& EntityData::GetAllComponentsMutable()
{
	return m_components;
}

EntityData* EntityData::GetParentMutable()
{
	if (ECS::IsValidID(m_parentId)) 
		return m_registry->TryGetComponentMutable<EntityData>(m_parentId);
	else return nullptr;
}
const EntityData* EntityData::GetParent() const
{
	if (ECS::IsValidID(m_parentId))
		return m_registry->TryGetComponent<EntityData>(m_parentId);
	else return nullptr;
}
bool EntityData::HasParent() const
{
	return ECS::IsValidID(m_parentId);
}
EntityData* EntityData::GetHighestParentMutable()
{
	EntityData* currentParent = GetParentMutable();
	if (currentParent == nullptr) return nullptr;

	EntityData* nextParent = currentParent->GetParentMutable();
	while (nextParent != nullptr)
	{
		currentParent = nextParent;
		nextParent = currentParent->GetParentMutable();
	}
	return currentParent;
}
const EntityData* EntityData::GetHighestParent() const
{
	const EntityData* currentParent = GetParent();
	if (currentParent == nullptr) return nullptr;

	const EntityData* nextParent = currentParent->GetParent();
	while (nextParent != nullptr)
	{
		currentParent = nextParent;
		nextParent = currentParent->GetParent();
	}
	return currentParent;
}

size_t EntityData::GetChildCount() const { return m_childrenIds.size(); }

EntityData& EntityData::CreateChild(const std::string& name, const TransformData& transform)
{
	EntityData& createdEntity= m_registry->CreateNewEntity(name, transform);
	PushChild(createdEntity);
	return createdEntity;
}
EntityData& EntityData::CreateChild(const std::string& name)
{
	return CreateChild(name, TransformData());
}

size_t EntityData::PushChild(EntityData& entity)
{
	size_t existingChildIndex = GetChildIndex(entity.GetId());
	if (existingChildIndex != size_t(-1)) return existingChildIndex;

	ECS::EntityID childId = m_childrenIds.emplace_back(entity.GetId());
	entity.m_parentId = GetId();
	if (m_SceneName != "") entity.m_SceneName = m_SceneName;

	//Note: we store index before event in case event triggers additional
	//children creation that might ruin the result of this function
	const size_t childIndex = GetChildCount() - 1;

	EntityData* currEntity = this;
	while (currEntity->GetParent() != nullptr)
	{
		currEntity = currEntity->GetParentMutable();
	}
	currEntity->m_OnFarthestChildElementAttached.Invoke(this);
	return childIndex;
}
EntityData* EntityData::TryPopChild(const ECS::EntityID id) { return TryPopChildAs<EntityData>(id); }
EntityData* EntityData::TryPopChildAt(const size_t index) { return TryPopChildAtAs<EntityData>(index); }
std::vector<EntityData*> EntityData::TryPopChildren(const size_t& startIndex, const size_t& count)
{
	std::vector<EntityData*> poppedChildren = {};
	for (size_t i = std::min(startIndex + count, m_childrenIds.size()) - 1; i >= startIndex; i--)
	{
		poppedChildren.push_back(TryPopChildAt(i));
	}
	return poppedChildren;
}
std::vector<EntityData*> EntityData::PopAllChildren()
{
	if (m_childrenIds.empty()) return {};

	std::vector<EntityData*> childrenCopy = {};
	childrenCopy.reserve(m_childrenIds.size());
	for (int i = m_childrenIds.size() - 1; i >= 0; i--)
	{
		childrenCopy.emplace_back(TryPopChildAt(i));
	}
	return childrenCopy;
}

std::vector<EntityData*> EntityData::GetChildrenMutable() 
{ 
	if (m_childrenIds.empty()) return {};

	std::vector<EntityData*> children = {};
	children.reserve(m_childrenIds.size());
	for (auto& childId : m_childrenIds)
	{
		children.emplace_back(m_registry->TryGetEntityMutable(childId));
	}
	return children;
}
std::vector<const EntityData*> EntityData::GetChildren() const
{
	if (m_childrenIds.empty()) return {};

	std::vector<const EntityData*> children = {};
	children.reserve(m_childrenIds.size());
	for (const auto& childId : m_childrenIds)
	{
		children.emplace_back(m_registry->TryGetEntity(childId));
	}
	return children;
}

EntityData* EntityData::TryGetChildEntityAtMutable(const size_t index)
{
	return TryGetChildComponentAtMutable<EntityData>(index);
}
EntityData* EntityData::TryGetChildEntityMutable(const ECS::EntityID id)
{
	return TryGetChildComponentMutable<EntityData>(id);
}
EntityData* EntityData::FindEntityRecursiveMutable(const ECS::EntityID id)
{
	return FindComponentRecursiveMutable<EntityData>(id);
}
EntityData* EntityData::FindParentEntityRecursiveMutable(const ECS::EntityID id, size_t* childIndex)
{
	return FindParentComponentRecursiveMutable<EntityData>(id, childIndex);
}

bool EntityData::HasChild(const ECS::EntityID id) const
{
	for (const auto& childId : m_childrenIds)
	{
		if (childId == id)
			return true;
	}
	return false;
}

size_t EntityData::GetChildIndex(const ECS::EntityID id) const
{
	if (m_childrenIds.empty()) return -1;

	for (size_t i = 0; i < m_childrenIds.size(); i++)
	{
		if (m_childrenIds[i] == id)
			return i;
	}
	return -1;
}

//std::vector<std::string> EntityData::GetDependencyFlags() const
//{
//	return {};
//}
void EntityData::InitFields()
{
	m_Fields = {};
}

std::string EntityData::ToString(const ECS::EntityID& id)
{
	return std::to_string(static_cast<uint32_t>(id));
}
std::string EntityData::ToString() const
{
	std::string componentNames = "Components: ";
	if (!m_components.empty())
	{
		for (size_t i = 0; i < m_components.size(); i++)
		{
			if (i != 0) componentNames += ", ";
			componentNames += FormatComponentName(typeid(*m_components[i]));
		}
	}
	return std::format("['{}'(ID:{} A:{})-> {}]", m_Name, ToString(m_id), std::to_string(m_isActive), componentNames);
}

void EntityData::Deserialize(const Json& json)
{
	//TODO: implement
	return;
}
Json EntityData::Serialize()
{
	//TODO: implement
	return {};
}


bool EntityData::operator==(const EntityData& other) const
{
	return m_id == other.m_id && m_Name == other.m_Name 
		&& m_SceneName == other.m_SceneName;
}
