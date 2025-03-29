#include "pch.hpp"
#include "FontData.hpp"

FontData::FontData() : FontData(0, {}) {}

FontData::FontData(const float& fontSize, const Font& font) :
	m_Font(font), m_FontSize(fontSize) {}
