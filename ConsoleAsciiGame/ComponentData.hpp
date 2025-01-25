#pragma once

namespace ECS
{
	class Entity;
}

struct ComponentData
{
	//TODOL: also add a static flag so we can optimize some components
	bool m_MutatedThisFrame;

	//Guaranteed to not be nullptr (but cant be a ref to allow it to be set
	//not on construction without the need to have it as constructor arg
	ECS::Entity* m_Entity;

	ComponentData();
	ECS::Entity& GetEntitySafe();
};