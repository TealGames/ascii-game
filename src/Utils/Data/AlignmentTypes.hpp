#pragma once
#include <cstdint>

using IntegralAlignType = std::uint8_t;
enum class AlignType : IntegralAlignType
{
    Align4 = 4,
	Align16 = 16
};