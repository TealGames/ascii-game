#include "pch.hpp"
#include "TextBuffer.hpp"
#include "HelperFunctions.hpp"

FontData::FontData(const std::uint8_t& fontSize, const Font& font) : 
	m_Font(&font), m_FontSize(fontSize) {}

TextBufferPosition::TextBufferPosition(const Utils::Point2D& pos, const TextChar& textChar,
	const Font& font, const std::uint8_t& fontSize)
	: m_Pos(pos), m_FontData(fontSize, font), m_Text(textChar)
{

}

std::string TextBufferPosition::ToString() const
{
	return std::format("[Pos:{}, Data:{}]",
		m_Pos.ToString(), m_Text.ToString());
}

TextBuffer::TextBuffer(const FontData& font, const std::vector<TextCharPosition>& positions) : 
	m_FontData(font), m_TextPositions(positions)
{
}

std::string ToString(const TextBufferMixed& buffer)
{
	return Utils::ToStringIterable<TextBufferMixed, TextBufferPosition>(buffer);
}