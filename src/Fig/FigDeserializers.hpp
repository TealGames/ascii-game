#pragma once
#include <string>

//TODO: these should probably not use string but should be changed into having a fig-related class
bool ToBool(const std::string& value);
int ToInt32(const std::string& value);
float ToFloat(const std::string& value);
std::uint8_t ToUint8(const std::string& value);