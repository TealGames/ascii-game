#include "pch.hpp"
#include "Utils/StringUtil.hpp"

namespace Utils
{
	StringUtil::StringUtil(const std::string& s) : str(s) {}

	StringUtil& StringUtil::TrimChar(const char c)
	{
		//size_t startIndex = str.find_first_not_of(' ');
		//size_t endIndex = str.find_last_not_of(' ');
		size_t startIndex = str.find_first_not_of(c);
		size_t endIndex = str.find_last_not_of(c);

		//If we did not find any for it, we just return early with no changes
		if (startIndex < 0 || startIndex >= str.size() ||
			endIndex < 0 || endIndex >= str.size()) return *this;

		str = str.substr(startIndex, endIndex - startIndex + 1);
		return *this;
	}

	StringUtil& StringUtil::TrimSpaces()
	{
		return TrimChar(' ');
	}

	StringUtil& StringUtil::TrimIdents()
	{
		return TrimChar('\t');
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

	StringUtil& StringUtil::RemoveChar(char c)
	{
		str.erase(std::remove(str.begin(), str.end(), c), str.end());
		return *this;
	}

	StringUtil& StringUtil::RemoveSpaces()
	{
		//Note: we could call RemoveChar(' ') however it would NOT work the same because
		//this erase considers tab spaces, but remove char would only consider single spaces 
		//and would leave tab spaces
		str.erase(std::remove_if(str.begin(), str.end(), isspace), str.end());
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