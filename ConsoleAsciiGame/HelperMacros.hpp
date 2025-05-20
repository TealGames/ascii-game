#pragma once
#include <type_traits>

#define FLAG_ENUM_OPERATORS(Enum)\
constexpr Enum operator|(Enum lhs, Enum rhs) { \
    using T = std::underlying_type_t<Enum>; \
    return static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs)); \
} \
constexpr Enum operator&(Enum lhs, Enum rhs) { \
    using T = std::underlying_type_t<Enum>; \
    return static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs)); \
} \
constexpr Enum operator^(Enum lhs, Enum rhs) { \
    using T = std::underlying_type_t<Enum>; \
    return static_cast<Enum>(static_cast<T>(lhs) ^ static_cast<T>(rhs)); \
} \
constexpr Enum operator~(Enum rhs) { \
    using T = std::underlying_type_t<Enum>; \
    return static_cast<Enum>(~static_cast<T>(rhs)); \
} \
constexpr Enum& operator|=(Enum& lhs, Enum rhs) { lhs = lhs | rhs; return lhs; } \
constexpr Enum& operator&=(Enum& lhs, Enum rhs) { lhs = lhs & rhs; return lhs; } \
constexpr Enum& operator^=(Enum& lhs, Enum rhs) { lhs = lhs ^ rhs; return lhs; }

