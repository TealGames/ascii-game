#include "pch.hpp"
#include "ECS/Entity/EntityRegistry.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "ECS/Component/Types/World/TransformData.hpp"

namespace ECS
{
	EntityRegistry::EntityRegistry() : m_registry() {}

	entt::registry& EntityRegistry::GetInternalRegistry() { return m_registry; }

	bool EntityRegistry::IsValidID(const EntityID& id) const
	{
		return m_registry.valid(id);
		auto view= m_registry.view<TransformData>();
	}
	EntityData& EntityRegistry::CreateNewEntity(const std::string& name, const TransformData& transformData)
	{
		EntityID id = m_registry.create();
		EntityData& createdEntity= AddComponent<EntityData>(id, EntityData(*this, id, name));
		//Note: we HAVE to add the transform via the entity because although entity data does not require an entity ptr,
		//the transform and all other components do
		createdEntity.AddComponent<TransformData>(transformData);

		return createdEntity;
	}


	EntityData* EntityRegistry::TryGetEntityMutable(const EntityID id)
	{
		return TryGetComponentMutable<EntityData>(id);
	}
	const EntityData* EntityRegistry::TryGetEntity(const EntityID id) const
	{
		return TryGetComponent<EntityData>(id);
	}
	bool EntityRegistry::HasEntity(const EntityID& id) const
	{
		return TryGetEntity(id) != nullptr;
	}
}
