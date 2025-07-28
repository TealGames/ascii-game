#pragma once
#include <concepts>
#include <ranges>

template <typename T>
concept IS_ITERABLE = requires(T t) {
    { std::begin(t) } -> std::input_or_output_iterator;
    { std::end(t) };
};