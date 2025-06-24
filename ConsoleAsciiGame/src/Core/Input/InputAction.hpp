#pragma once
#include "raylib.h"
#include <string>
#include <vector>
#include "Core/Input/InputState.hpp"
#include "Core/Input/InputKey.hpp"

namespace Input
{
	class InputAction
	{
	private:
	public:
		std::string m_Name;
		std::vector<const InputKey*> m_Keys;

	private:
	public:
		InputAction(const std::string& name, const std::vector<const InputKey*>& keys);

		bool IsDown() const;
		bool IsPressed() const;
		bool IsReleased() const;

		std::string ToString() const;
	};
}


