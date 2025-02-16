#pragma once
#include <string>

namespace Utils {
	class StringUtil {
	private:
		std::string str;
	public:
		StringUtil(const std::string&);
		StringUtil& Trim();
		StringUtil& ToLowerCase();
		StringUtil& RemoveChar(char c);
		StringUtil& RemoveSpaces();

		std::string ToString();
		explicit operator std::string();
	};
}

