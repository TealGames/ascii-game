#pragma once
#include <vector>
#include "Utils/HelperMacros.hpp"
#include "ECS/Component/ComponentField.hpp"
#include "Core/Serialization/IJsonSerializable.hpp"
#include "Core/IValidateable.hpp"
#include "ECS/Entity/EntityID.hpp"

namespace Engine { class TransformComponent; }
namespace Engine::Serialization
{
	class Serializer;
	class Deserializer;
}
namespace Engine::ECS
{
	enum class HighestDependecyLevel : std::uint8_t
	{
		/// <summary>
		/// This means the component does not depend on any other outside data
		/// </summary>
		None,
		/// <summary>
		/// This means the component requires another SIBLING component 
		/// (a component on the same entity as this component)
		/// </summary>
		SiblingComponent,
		/// <summary>
		/// This means the component requires ether a component on ANOTHER entity
		/// or the full entity object
		/// </summary>
		Entity,
	};

	enum class ComponentStateFlag : std::uint8_t
	{
		None = 0,
		EntityActive = 1,
		EntityInactive = 1 << 1,
		EntitySerializable = 1 << 2,
		EntityUnserializable = 1 << 3,
		EntityImmovable = 1 << 4,
		EntityMovable = 1 << 5,
		ComponentEnabled = 1 << 6,
		ComponentDisbled = 1 << 7,
		All = 0xFF
	};
	FLAG_ENUM_OPERATORS(ComponentStateFlag)
		std::string ToString(const ComponentStateFlag flags);

	inline constexpr ComponentStateFlag ANY_ENTITY_ACTIVE_FLAG = ComponentStateFlag::EntityActive | ComponentStateFlag::EntityInactive;
	inline constexpr ComponentStateFlag ANY_SERIALIZABLE_FLAG = ComponentStateFlag::EntitySerializable | ComponentStateFlag::EntityUnserializable;
	inline constexpr ComponentStateFlag ANY_MOVABLE_FLAG = ComponentStateFlag::EntityImmovable | ComponentStateFlag::EntityMovable;
	inline constexpr ComponentStateFlag ANY_COMPONENT_ENABLED_FLAG = ComponentStateFlag::ComponentEnabled | ComponentStateFlag::ComponentDisbled;

	inline constexpr ComponentStateFlag ALL_ACTIVE_ENABLED_FLAG = ComponentStateFlag::EntityActive | ANY_SERIALIZABLE_FLAG |
		ANY_MOVABLE_FLAG | ComponentStateFlag::ComponentEnabled;

	/// <summary>
	/// This means the component requires ether a component on ANOTHER entity
	/// or the full entity object
	/// </summary>
	constexpr const char* ENTITY_DEPENDENCY_FLAG = "Entity";

	class EntityData;
	using DirtyFlag = std::uint8_t;

	class Component
	{
	private:
		//Guaranteed to not be nullptr (but cant be a ref to allow it to be set
		//not on construction without the need to have it as constructor arg
		EntityData* m_entity;

	protected:
		/// <summary>
		/// If true, signifies that this object has been modified.
		/// Useful for optimization (lazy initialization for example and using it as a flag for updating values
		/// only when necessary)
		/// </summary>
		mutable DirtyFlag m_dirtyFlags;
		//mutable bool m_isDirty;

	public:
		friend class EntityData;

		/// <summary>
		/// If true, component is enabled, otherwise it is disabled. 
		/// Note: this does not change anything directly as components need to check this 
		/// flag themselves
		/// </summary>
		bool m_IsEnabled;
		//TODO: the fields for a component should be placed into a registry
		std::vector<ComponentField> m_Fields;

		std::function<void(DirtyFlag)> m_DirtyCallback;

	private:
	public:
		Component();
		virtual ~Component() = default;

		EntityData& GetEntityMutable();
		const EntityData& GetEntity() const;
		TransformComponent& GetTransformMutable();
		const TransformComponent& GetTransform() const;

		ECS::EntityID GetEntityID() const;
		/// <summary>
		/// Returns true if this component is active and the entity is active
		/// </summary>
		/// <returns></returns>
		bool IsInActiveAndEnabledState() const;

		bool IsDirty() const;
		/*void SetAllDirtyFlags() const;
		void SetAllCleanFlags() const;*/
		void SetDirtyFlag(const DirtyFlag flag) const;
		bool HasDirtyFlag(const DirtyFlag flag) const;
		void SetAllFlagsDirty(const bool isDirty) const;
		ComponentStateFlag GetStateFlags() const;

		std::vector<ComponentField>& GetFieldsMutable();
		/// <summary>
		/// A function that creates the fields that can be used by outside sources.
		/// Note: this must be separate from constructor because base class constructor defaults initializes them
		/// (to prevent derived setting it and being reverted by base class constructor) and because polymorphism
		/// is not established until AFTER constructors are done (therefore base class cannot invoke it)
		/// </summary>
		virtual void InitFields();
		const std::vector<ComponentField>& GetFields() const;
		ComponentField* TryGetFieldMutable(const std::string& name);
		const ComponentField* TryGetField(const std::string& name) const;
		std::string ToStringFields() const;

		virtual void Serialize(Serialization::Serializer& serializer) const = 0;
		virtual void Deserialize(Serialization::Deserializer& deserializer) = 0;

		virtual bool Validate();

		virtual std::string ToString() const;
	};

	std::string FormatComponentName(const Component* component);
	std::string FormatComponentName(const std::type_info& typeInfo);

	template<typename... ComponentTs>
	std::vector<const std::type_info*> CreateComponentTypes()
	{
		return { (&typeid(ComponentTs))... };
	}
}