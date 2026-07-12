#pragma once
#include "Core/Primitives/NormalizedValue.hpp"

namespace Engine::UI
{
	struct UIPadding
	{
		NormalizedValue m_Left;
		NormalizedValue m_Right;
		NormalizedValue m_Top;
		NormalizedValue m_Bottom;

		UIPadding(const float& left = 0, const float& right = 0, const float& top = 0, const float& bottom = 0);

		void SetAll(const float value);
		bool HasNoPadding() const;

		std::string ToString() const;
	};
}


