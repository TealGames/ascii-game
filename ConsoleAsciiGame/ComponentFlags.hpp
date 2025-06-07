#pragma once
#include "HelperMacros.hpp"

using ComponentFlagsType = std::uint32_t;
enum class ComponentFlags : ComponentFlagsType
{
	Transform		= 1 << 0,
	Renderer		= 1 << 1,
	Camera			= 1 << 2,
	CollisionBox	= 1 << 3,
	PhysicsBody		= 1 << 4,
	Trigger			= 1 << 5,
	Input			= 1 << 6,
	Player			= 1 << 7,
	LightSource		= 1 << 8,
	ParticleEmitter	= 1 << 9,
	Animator		= 1 << 10,
	SpriteAnimator	= 1 << 11,

	UITransform		= 1 << 12,
	UIRenderer		= 1 << 13,
	UILayout		= 1 << 14,
	UISelectable	= 1 << 15,
	UIText			= 1 << 16,
	UIToggle		= 1 << 17,
	UIButton		= 1 << 18,
	UIInputField	= 1 << 19,
	UISlider		= 1 << 20,
	UIColorPicker	= 1 << 21,
	UIContainer		= 1 << 22,
	UIPanel			= 1 << 23,
};
FLAG_ENUM_OPERATORS(ComponentFlags)


