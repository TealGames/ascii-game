#include "pch.hpp"
#include "ECS/Component/Component.hpp"
#include "Utils/Debug.hpp"
#include "ECS/Component/Types/World/EntityData.hpp"
#include "Utils/ToStringFunctions.hpp"

namespace Engine::ECS
{
	std::string ToString(const ComponentStateFlag flags)
	{
		if (flags == ComponentStateFlag::None)
			return "None";

		std::string result = "";
		if ((flags & ComponentStateFlag::EntityActive) != 0)
			result += "[EntityACTIVE]";
		if ((flags & ComponentStateFlag::EntityInactive) != 0)
			result += "[EntityINACTIVE]";
		if ((flags & ComponentStateFlag::EntitySerializable) != 0)
			result += "[EntitySERIALIZABLE]";
		if ((flags & ComponentStateFlag::EntityUnserializable) != 0)
			result += "[EntityUNSERIALIZABLE]";
		if ((flags & ComponentStateFlag::EntityImmovable) != 0)
			result += "[EntityIMMOVABLE]";
		if ((flags & ComponentStateFlag::EntityMovable) != 0)
			result += "[EntityMOVABLE]";
		if ((flags & ComponentStateFlag::ComponentEnabled) != 0)
			result += "[ComponentENABLED]";
		if ((flags & ComponentStateFlag::ComponentDisbled) != 0)
			result += "[ComponentDISABLED]";
		return result;
	}

	Component::Component()
		: m_dirtyFlags(), m_IsEnabled(true), m_entity(nullptr), m_Fields() //m_dependencyLevel(dependency)
	{
	}

	EntityData& Component::GetEntityMutable()
	{
		ENGINE_ASSERT(m_entity != nullptr, "Tried to retrieve entity from component MUTABLE safely but it is NULLPTR "
			"(it means a function creating or adding component probably did not update this setting)");
		return *m_entity;
	}

	const EntityData& Component::GetEntity() const
	{
		ENGINE_ASSERT(m_entity != nullptr, "Tried to retrieve entity from component safely but it is NULLPTR "
			"(it means a function creating or adding component probably did not update this setting)");
		return *m_entity;
	}

	TransformComponent& Component::GetTransformMutable() { return GetEntityMutable().GetTransformMutable(); }
	const TransformComponent& Component::GetTransform() const { return GetEntity().GetTransform(); }
	ECS::EntityID Component::GetEntityID() const { return GetEntity().GetId(); }

	bool Component::IsInActiveAndEnabledState() const
	{
		return m_IsEnabled && GetEntity().IsEntityActive();
	}
	bool Component::IsDirty() const { return m_dirtyFlags != 0; }
	bool Component::HasDirtyFlag(const DirtyFlag flag) const
	{
		return (m_dirtyFlags & flag) != 0;
	}
	void Component::SetDirtyFlag(const DirtyFlag flag) const
	{
		m_dirtyFlags |= flag;
		if (m_DirtyCallback != nullptr)
			m_DirtyCallback(m_dirtyFlags);
	}
	void Component::SetAllFlagsDirty(const bool isDirty) const
	{
		if (isDirty)
		{
			m_dirtyFlags = ~0;
			if (m_DirtyCallback != nullptr)
				m_DirtyCallback(m_dirtyFlags);
		}
		else m_dirtyFlags = 0;
	}
	ComponentStateFlag Component::GetStateFlags() const
	{
		ComponentStateFlag flags = ComponentStateFlag::None;

		const EntityData& entity = GetEntity();
		if (entity.IsEntityActive()) flags |= ComponentStateFlag::EntityActive;
		else flags |= ComponentStateFlag::EntityInactive;

		if (entity.m_IsSerializable) flags |= ComponentStateFlag::EntitySerializable;
		else flags |= ComponentStateFlag::EntityUnserializable;

		if (entity.m_IsImmovable) flags |= ComponentStateFlag::EntityImmovable;
		else flags |= ComponentStateFlag::EntityMovable;

		if (m_IsEnabled) flags |= ComponentStateFlag::ComponentEnabled;
		else flags |= ComponentStateFlag::ComponentDisbled;

		return flags;
	}
	//void Component::SetDirty(const bool isDirty)
	//{
	//	m_isDirty = isDirty;
	//}

	void Component::InitFields()
	{
		return;
	}

	std::vector<ComponentField>& Component::GetFieldsMutable()
	{
		return m_Fields;
	}

	const std::vector<ComponentField>& Component::GetFields() const
	{
		return m_Fields;
	}

	ComponentField* Component::TryGetFieldMutable(const std::string& name)
	{
		for (auto& field : m_Fields)
		{
			if (field.m_FieldName == name)
				return &field;
		}
		return nullptr;
	}
	const ComponentField* Component::TryGetField(const std::string& name) const
	{
		for (const auto& field : m_Fields)
		{
			if (field.m_FieldName == name)
				return &field;
		}
		return nullptr;
	}

	std::string Component::ToStringFields() const
	{
		std::vector<std::string> fieldStrings = {};
		for (const auto& field : m_Fields)
		{
			fieldStrings.emplace_back(field.ToString());
		}
		return ::Utils::ToStringIterable(fieldStrings);
	}

	bool Component::Validate()
	{
		return true;
	}

	std::string Component::ToString() const
	{
		return std::format("[BaseComp Entity:{}]", GetEntity().m_Name);
	}

	std::string FormatComponentName(const Component* component)
	{
		return FormatComponentName(typeid(*component));
	}
	std::string FormatComponentName(const std::type_info& typeInfo)
	{
		std::string typeName = ::Utils::FormatTypeName(typeInfo.name());
		size_t index = typeName.find("Data");
		if (index != std::string::npos) typeName = typeName.substr(0, index);

		index = typeName.find("Component");
		if (index != std::string::npos) typeName = typeName.substr(0, index);

		return typeName;
	}
}