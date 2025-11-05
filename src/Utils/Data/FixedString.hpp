#pragma once
#include <cstdint>
#include <concepts>
#include <array>
#include <string>
#include <string_view>
#include <format>
#include "Utils/Debug.hpp"

template<std::uint32_t N>
requires (N > 0)
class FixedString
{
private:
    using SizeType = decltype([] 
        {
            if constexpr (N > 65535)
                return std::uint32_t();
            else if constexpr (N > 256)
                return std::uint16_t();
            else
                return std::uint8_t();
        }());

    char m_chars[N];
    SizeType m_usedSize;
public:

private:
    void Set(const std::string& str)
    {
        if (str.size() > N)
        {
            LogError(std::format("Attempted to create fixed string with std::string:{} size({}) "
                "greater than supported size:{}", str, str.size(), N));
            return;
        }

        m_usedSize = str.size();
        memcpy(&m_chars, &str[0], m_usedSize);
    }
    void Set(const char* c, const std::uint32_t size)
    {
        if (c == nullptr)
            return;

        if (size > N)
        {
            LogError(std::format("Attempted to create fixed string with char*:{} size({}) "
                "greater than supported size:{}", c, size, N));
            return;
        }

        m_usedSize = size;
        memcpy(&m_chars, c, size);
    }
    void Set(const char* c)
    {
        if (c == nullptr)
            return;

        const char* currentC = c;
        std::uint32_t size = 0;
        while (*currentC != '\0')
        {
            currentC++;
            size++;
        }
        Set(c, size);
    }

public:
	FixedString() : m_chars({}), m_usedSize(0) {}
    FixedString(const std::string& str) : FixedString()
    {
        Set(str);
    }
    FixedString(const char* c, const std::uint32_t size) : FixedString()
    {
        Set(c, size);
    }
    FixedString(const char* c) : FixedString()
    {
        Set(c);
    }
    FixedString(const std::array<char, N>& arr) : FixedString(&arr[0], N) {}
    FixedString(const std::string_view& view) : FixedString(view.data(), view.size()) {}
   
    std::uint32_t GetUsedSize() const { return m_usedSize; }
    std::uint32_t GetCapacity() const { return N; }

    bool Empty() const { return m_usedSize == 0; }

    char& operator[](const std::uint32_t index) noexcept
    {
        return m_chars[index];
    }
    const char& operator[](const std::uint32_t index) const noexcept
    {
        return m_chars[index];
    }

    char GetCharAtUnsafe(const std::uint32_t index) const { return m_chars[index]; }
    char GetCharAt(const std::uint32_t index) const
    {
        if(index >= N)
        {
            LogError("GetCharAt invoked with out of bounds index");
            return '\0';
        }
        return m_chars[index];
    }

    bool Contains(const char c) const
    {
        if (Empty())
            return false;

        for (int i = 0; i < m_usedSize; i++)
        {
            if (m_chars[i] == c)
                return true;
        }
        return false;
    }

    const char* GetMemPointer() const { return &m_chars[0]; }

    template<std::uint32_t OTHER_N>
    requires (OTHER_N > 0)
    FixedString<OTHER_N> Substr(const std::uint32_t initialIndex, const std::uint32_t size) const
    {
        if (initialIndex >= N)
        {
            LogError("Substr invoked with out of bounds index");
            return FixedString<OTHER_N>();
        }

        return FixedString<OTHER_N>(&m_chars[initialIndex], size);
    }

    explicit operator std::string() const { return std::string(GetMemPointer(), m_usedSize); }
    explicit operator std::string_view() const { return std::string_view(GetMemPointer(), m_usedSize); }

    bool StringEquals(const char* buffer, const size_t length) const
    {
        return m_usedSize == length && memcmp(GetMemPointer(), buffer, m_usedSize) == 0;
    }
    bool StringEquals(const std::string_view& view) const
    {
        return StringEquals(view.data(), view.size());
    }

    template<std::uint32_t OTHER_N>
    bool operator==(const FixedString<OTHER_N>& other) const
    {
        return StringEquals(other.GetMemPointer(), other.GetUsedSize());
    }

    std::string ToString() const
    {
        return (std::string)(*this);
    }
    std::string_view ToStringView() const
    {
        return (std::string_view)(*this);
    }
    const char* ToCStr() const
    {
        return &m_chars[0];
    }
};

namespace std
{
    template<std::uint32_t N> 
    struct hash<FixedString<N>>
    {
        std::size_t operator()(const FixedString<N>& str) const noexcept
        {
            const std::string_view view(str.GetMemPointer(), str.GetCapacity());
            return std::hash<std::string_view>{}(view);
        }
    };

    template<std::uint32_t N>
    struct formatter<FixedString<N>> : formatter<const char*>
    {
        template<typename FormatContext>
        auto format(const FixedString<N>& str, FormatContext& ctx) const
        {
            return formatter<const char*>::format(str.GetMemPointer(), ctx);
        }
    };
}

using String8 = FixedString<8>;
using String16 = FixedString<16>;
using String32 = FixedString<32>;
using String64 = FixedString<64>;

bool ContainsChar(const char* buffer, const size_t length, const char c);
size_t FindCharIndex(const char* buffer, const size_t length, const char c);

template<size_t N>
bool StringEqualsAny(const std::string_view stringView, const FixedString<N>** compareStrings, const size_t compareStringLength)
{
    if (compareStrings == nullptr)
        return false;

    for (int i=0; i<compareStringLength; i++)
    {
        if (compareStrings[i]->StringEquals(stringView))
            return true;
    }
    return false;
}