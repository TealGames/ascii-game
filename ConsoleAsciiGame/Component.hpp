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
	protected:
		/// <summary>
		/// If true, it means this component has had changes done during the current frame
		/// that are different from the state held the previous frame
		/// otherwise it has not. This is useful for optimizing and reducing update calls
		/// and/or rendering calls if there is no dirty objects
		/// </summary>
		bool m_isDirty;
		
	public:
		virtual void Start() = 0;
		virtual void UpdateStart(float deltaTime) = 0;
		virtual void UpdateEnd(float deltaTime) = 0;

		bool IsDirty() const;

	public:
		Component();
		~Component();

		virtual UpdatePriority GetUpdatePriority() const;
	};
}

