#include "Utils/Data/FixedString.hpp"

bool ContainsChar(const char* buffer, const size_t length, const char c)
{
    for (int i = 0; i < length; i++)
    {
        if (buffer[i] == c)
            return true;
    }
    return false;
}

size_t FindCharIndex(const char* buffer, const size_t length, const char c)
{
    for (int i = 0; i < length; i++)
    {
        if (buffer[i] == c)
            return i;
    }
    return std::string::npos;
}