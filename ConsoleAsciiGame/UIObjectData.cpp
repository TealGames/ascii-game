#include "pch.hpp"
#include "UIObjectData.hpp"

UIObjectData::UIObjectData() : UIObjectData(NormalizedPosition()) {}

UIObjectData::UIObjectData(const NormalizedPosition& normalizedPos) :
	m_normalizedPos(normalizedPos) 
{

}

const NormalizedPosition& UIObjectData::GetNormalizedPos() const
{
	return m_normalizedPos;
}

void UIObjectData::InitFields()
{
	m_Fields = {};
}