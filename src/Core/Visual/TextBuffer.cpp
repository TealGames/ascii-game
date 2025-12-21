#include "pch.hpp"
#include "Core/Visual/TextBuffer.hpp"
#include "Utils/HelperFunctions.hpp"

TextBufferCharPosition2D::TextBufferCharPosition2D() : TextBufferCharPosition2D({}, {}, {}) {}
TextBufferCharPosition2D::TextBufferCharPosition2D(const WorldPosition2D& pos, const TextChar& textChar, const WorldFontProperties& font)
	: m_Pos(pos), m_Text(textChar), m_FontData(font) {}

Vec2 TextBufferCharPosition2D::GetWorldSize() const
{
	return m_FontData.m_RectSize;
}
std::string TextBufferCharPosition2D::ToString() const
{
	return std::format("[VisualDataPosChar2D Pos:{} Text:{}]", m_Pos.ToString(), m_Text.ToString());
}

TextBufferCharPosition3D::TextBufferCharPosition3D() : TextBufferCharPosition3D({}, {}, {}) {}
TextBufferCharPosition3D::TextBufferCharPosition3D(const WorldPosition3D& pos, const TextChar& textChar, const WorldFontProperties& font)
	: m_Pos(pos), m_Text(textChar), m_FontData(font) {}

Vec2 TextBufferCharPosition3D::GetWorldSize() const
{
	return m_FontData.m_RectSize;
}
std::string TextBufferCharPosition3D::ToString() const
{
	return std::format("[VisualDataPosChar3D Pos:{} Text:{}]", m_Pos.ToString(), m_Text.ToString());
}

std::string ToString(const std::vector<TextBufferCharPosition2D>& chars)
{
	std::vector<std::string> strs = {};
	for (const auto& c : chars)
	{
		strs.push_back(c.ToString());
	}
	return Utils::ToStringIterable(strs);
}

TextBufferChar::TextBufferChar() : TextBufferChar({}, {}) {}

TextBufferChar::TextBufferChar(const TextChar& textChar, const WorldFontProperties& font)
	: m_Text(textChar), m_FontData(font) {}

Vec2 TextBufferChar::GetWorldSize() const
{
	return m_FontData.m_RectSize;
}
std::string TextBufferChar::ToString() const
{
	return std::format("[VisualDataChar Text:{}]", m_Text.ToString());
}