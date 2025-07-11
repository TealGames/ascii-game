#pragma once
#include <string>

namespace Utils {
	class StringUtil {
	private:
		std::string str;
	public:
		StringUtil(const std::string&);
		StringUtil& TrimSpaces();
		StringUtil& TrimIdents();
		StringUtil& TrimChar(const char c);

		StringUtil& ToLowerCase();
		StringUtil& RemoveChar(char c);
		StringUtil& RemoveSpaces();

		std::string ToString();
		explicit operator std::string();
	};
}

