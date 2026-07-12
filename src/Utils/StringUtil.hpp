#pragma once
#include <string>

namespace Utils 
{
	class StringUtil 
	{
	private:
		std::string str;
	public:
		StringUtil(const std::string&);
		/// <summary>
		/// Removes only the space ' ' char from the front and end
		/// of the string until a non-space char
		/// </summary>
		/// <returns></returns>
		StringUtil& TrimSpaces();

		/// <summary>
		/// Removes only the tab '\t' char from the front and end
		/// of the string until a non-tab char
		/// </summary>
		/// <returns></returns>
		StringUtil& TrimIdents();

		/// <summary>
		/// Removes the all chars until the first instance of `c` arg
		/// and all chars after the last insstance of `c` arg
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		StringUtil& TrimChar(const char c);

		/// <summary>
		/// Removes all ' ', '\t', '\n', '\r', '\f', '\v' chars 
		/// from the start and end of the string
		/// </summary>
		/// <returns></returns>
		StringUtil& TrimAnySpaceChar();

		StringUtil& ToLowerCase();
		StringUtil& RemoveChar(char c);
		StringUtil& RemoveSpaces();

		std::string ToString();
		explicit operator std::string();
	};
}

