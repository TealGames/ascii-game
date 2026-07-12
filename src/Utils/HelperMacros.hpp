#pragma once
#include <type_traits>

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

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
constexpr Enum& operator^=(Enum& lhs, Enum rhs) { lhs = lhs ^ rhs; return lhs; } \
constexpr bool operator==(Enum lhs, int type) { \
    return static_cast<std::underlying_type_t<Enum>>(lhs)== static_cast<std::underlying_type_t<Enum>>(type); \
} \
constexpr bool operator!=(Enum lhs, int type) { \
    return !(lhs == type); \
} \
constexpr bool operator==(Enum lhs, std::underlying_type_t<Enum> type) { \
    return static_cast<std::underlying_type_t<Enum>>(lhs)== type; \
} \
constexpr bool operator!=(Enum lhs, std::underlying_type_t<Enum> type) { \
    return !(lhs == type); \
} \

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

#define STATIC_ASSERT_HAS_FUNCTION_NAMED(FUNCTION, Type, ReturnType, ...) \
    static_assert(HasFunction##FUNCTION<Type, ReturnType, __VA_ARGS__>, \
                  "Type " #Type " does not have function " #FUNCTION " with the given signature");

#define STATIC_ASSERT_HAS_FUNCTION(FUNCTION, Type) \
    static_assert(HasFunction##FUNCTION<Type>, \
                  "Type " #Type " does not have function " #FUNCTION " with the given signature");

#define DEFINE_TEMPLATE_HAS_NAMED_FUNCTION(FUNCTION)                            \
template<typename T, typename ReturnType, typename... Args>                     \
concept HasNamedFunction##FUNCTION = requires(T t, Args... args) {              \
    { t.FUNCTION(args...) } -> std::convertible_to<ReturnType>;                 \
};

#define DEFINE_TEMPLATE_HAS_FUNCTION(FUNCTION, RETURN_TYPE)                     \
template<typename T>                                                            \
concept HasFunction##FUNCTION = requires(T t) {                                 \
    { t.FUNCTION() } -> std::convertible_to<RETURN_TYPE>;                       \
};

#define DEFINE_TEMPLATE_HAS_VOID_NAMED_FUNCTION(FUNCTION)                       \
template<typename T, typename... Args>                                          \
concept HasVoidNamedFunction##FUNCTION = requires(T t, Args... args) {          \
    { t.FUNCTION(args...) } -> std::same_as<void>;                              \
};

#define DEFINE_TEMPLATE_HAS_VOID_FUNCTION(FUNCTION)                             \
template<typename T>                                                            \
concept HasVoidFunction##FUNCTION = requires(T t) {                             \
    { t.FUNCTION() } -> std::same_as<void>;                                     \
};

#define DEFINE_TEMPLATE_HAS_FREE_NAMED_FUNCTION(FUNCTION)                       \
template<typename ReturnType, typename... Args>                                 \
concept HasFreeNamedFunction##FUNCTION = requires(Args... args) {               \
    { FUNCTION(args...) } -> std::convertible_to<ReturnType>;                   \
};

#define DEFINE_TEMPLATE_HAS_FREE_VOID_NAMED_FUNCTION(FUNCTION)                  \
template<typename... Args>                                                      \
concept HasFreeVoidNamedFunction##FUNCTION = requires(Args... args) {           \
    { FUNCTION(args...) } -> std::same_as<void>;                                \
};

#define DEFINE_TEMPLATE_HAS_PROPERTY(MEMBER, RETURN_TYPE)                               \
template<typename T>                                                                    \
concept HasProperty##MEMBER = requires(T t) {                                           \
    { t.MEMBER } -> std::convertible_to<RETURN_TYPE>;                                   \
};

#define DEFINE_TEMPLATE_INVOCABLE(NAME, RETURN_TYPE, ...)                                               \
template<typename T>                                                                                    \
concept NAME = requires(T t) {                                                                          \
    { std::invoke(t __VA_OPT__(, std::declval<__VA_ARGS__>())) } -> std::convertible_to<RETURN_TYPE>;   \
};

#define ARR_SIZE(ARR_NAME) sizeof(ARR_NAME) / sizeof(ARR_NAME[0])