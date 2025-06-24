#include "pch.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"

TextBufferCharPosition::TextBufferCharPosition() : TextBufferCharPosition({}, {}, {}) {}
TextBufferCharPosition::TextBufferCharPosition(const Vec2& pos, const TextChar& textChar, const FontProperties& font)
	: m_Pos(pos), m_Text(textChar), m_FontData(font) {}

Vec2 TextBufferCharPosition::GetWorldSize() const
{
	return m_Text.GetWorldSize(m_FontData);
}
std::string TextBufferCharPosition::ToString() const
{
	return std::format("[VisualDataPosChar Pos:{} Text:{}]", m_Pos.ToString(), m_Text.ToString());
}
std::string ToString(const std::vector<TextBufferCharPosition>& chars)
{
	std::vector<std::string> strs = {};
	for (const auto& c : chars)
	{
		strs.push_back(c.ToString());
	}
	return Utils::ToStringIterable<std::vector<std::string>, std::string>(strs);
}

TextBufferChar::TextBufferChar() : TextBufferChar({}, {}) {}

TextBufferChar::TextBufferChar(const TextChar& textChar, const FontProperties& font)
	: m_Text(textChar), m_FontData(font) {}

Vec2 TextBufferChar::GetWorldSize() const
{
	return m_Text.GetWorldSize(m_FontData);
}
std::string TextBufferChar::ToString() const
{
	return std::format("[VisualDataChar Text:{}]", m_Text.ToString());
}