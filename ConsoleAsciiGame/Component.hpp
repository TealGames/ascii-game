#pragma once
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>
#include "Updateable.hpp"

namespace ECS
{
	//enum class ComponentType
//{
//	LightSource,
//	AudioSource,
//};

//std::string ToString(const ComponentType& type);
	using UpdatePriority = short;

	/// <summary>
	/// Min Priority means the last one updates
	/// </summary>
	constexpr short MIN_PRIORITY = std::numeric_limits<short>::min();

	/// <summary>
	/// Max priority means the most priority- so first one updated
	/// </summary>
	constexpr short MAX_PRIORITY = std::numeric_limits<short>::max();

	class Component : public Updateable
	{
	private:

	public:
		virtual void Start() = 0;
		virtual void UpdateStart(float deltaTime) = 0;
		virtual void UpdateEnd(float deltaTime) = 0;

	public:
		Component();
		~Component();

		virtual UpdatePriority GetUpdatePriority() const;
	};
}

