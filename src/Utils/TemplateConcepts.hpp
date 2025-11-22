#pragma once
#include <concepts>
#include <ranges>
#include <type_traits>
#include "Utils/HelperMacros.hpp"

template <typename T>
concept IS_ITERABLE = requires(T t) {
    { std::begin(t) } -> std::input_or_output_iterator;
    { std::end(t) };
};

DEFINE_TEMPLATE_HAS_FUNCTION(ToString, std::string);

/// <summary>
/// Checks if an invocable type (functor, function ptr, lambda, std::function) 
/// has return type and arguments as specified
/// </summary>
template<typename TReturn, typename TFunc, typename ...Args>
concept IsInvocableType= std::is_invocable_r_v<TReturn, TFunc, Args...>;

/// <summary>
/// Checks if type IS invocable (is functor, function ptr, lambda or std::function)
/// by checking if it has () operator
/// </summary>
template<typename T>
concept IsInvocable = requires(T t) {
    &T::operator();
};

template<typename T, typename ...Args>
concept AllSame = (std::same_as<Args, T> && ...);
