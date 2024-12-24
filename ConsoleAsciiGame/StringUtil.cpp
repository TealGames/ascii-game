#include "StringUtil.hpp"
#include "algorithm"
#include <string>

namespace Utils
{
	StringUtil::StringUtil(const std::string& s) : str(s) {}

	StringUtil& StringUtil::Trim()
	{
		size_t startIndex = str.find_first_not_of(' ');
		size_t endIndex = str.find_last_not_of(' ');

		//If we did not find any for it, we just return early with no changes
		if (startIndex < 0 || startIndex >= str.size() ||
			endIndex < 0 || endIndex >= str.size()) return *this;

		str = str.substr(startIndex, endIndex - startIndex + 1);
		return *this;
	}

	StringUtil& StringUtil::ToLowerCase()
	{
		std::string result;
		for (auto& c : str)
		{
			result += (char)tolower(c);
		}
		str = result;
		return *this;
	}

	StringUtil& StringUtil::RemoveChar(const char& c)
	{
		str.erase(std::remove(str.begin(), str.end(), c), str.end());
		return *this;
	}


	std::string StringUtil::ToString()
	{
		return str;
	}

	StringUtil::operator std::string()
	{
		return ToString();
	}
}