#include <string>
#include <iostream>
#include <format>
#include <string_view>

template<typename ...Args>
std::string Print(std::string_view str, Args&&... args)
{
	const std::string strCreated = std::format(str, args...);
	std::cout << strCreated << std::endl;
	return strCreated;
}