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


//TODO: craete an extensible mutli-field use for arithmetic/logic operators
#define DECLARE_ARITHMETIC_OPERATORS(Type)				\
constexpr Type operator+(const Type& other) const;		\
constexpr Type operator-(const Type& other) const;		\
constexpr Type operator*(const Type& other) const;		\
constexpr Type operator*(const float& scalar) const;	\
constexpr Type operator/(const Type& other) const;		\
constexpr Type operator/(const float& scalar) const;	\

#define DECLARE_COMPOUND_ASSIGNMENT_OPERATORS(Type)     \
Type& operator+=(const Type& other);                    \
Type& operator-=(const Type& other);                    \
Type& operator*=(const Type& other);                    \
Type& operator/=(const Type& other);                    \

#define IMPLEMENT_COMPOUND_ASSIGNMENT_OPERATORS(Type)   \
Type& Type::operator+=(const Type& other)               \
{                                                       \
    *this = *this + other;                              \
    return *this;                                       \
}                                                       \
Type& Type::operator-=(const Type& other)               \
{                                                       \
    *this = *this - other;                              \
    return *this;                                       \
}                                                       \
Type& Type::operator*=(const Type& other)               \
{                                                       \
    *this = *this + other;                              \
    return *this;                                       \
}                                                       \
Type& Type::operator/=(const Type& other)               \
{                                                       \
    *this = *this / other;                              \
    return *this;                                       \
}                                                       \
