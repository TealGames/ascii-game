#pragma once
#include <concepts>
#include <ranges>
#include <type_traits>
#include <variant>
#include <unordered_map>
#include <tuple>
#include <memory>
#include "Utils/HelperMacros.hpp"

namespace Utils
{
    template <typename T>
    concept IsIterable = requires(T t) 
    {
        { std::begin(t) } -> std::input_or_output_iterator;
        { std::end(t) };
    };

    DEFINE_TEMPLATE_HAS_FUNCTION(ToString, std::string);
    DEFINE_TEMPLATE_HAS_NAMED_FUNCTION(ToString);
    DEFINE_TEMPLATE_HAS_FREE_NAMED_FUNCTION(ToString);

    template <typename T, typename = void>
    struct HasOstreamOperator : std::false_type {};
    template <typename T>
    struct HasOstreamOperator<T, std::void_t<decltype(std::declval<std::ostringstream&>() << std::declval<T>())>> : std::true_type {};

    template<typename TException>
    concept IsExceptionType = std::is_base_of_v<std::exception, TException>;
    /// <summary>
    /// Checks if an invocable type (functor, function ptr, lambda, std::function) 
    /// has return type and arguments as specified. NOTE: VOID IS FOR RETURN IS ALLOWED
    /// </summary>
    template<typename TReturn, typename TFunc, typename ...Args>
    concept IsInvocableType = std::is_invocable_r_v<TReturn, TFunc, Args...>;

    /// <summary>
    /// Checks if type IS invocable (is functor, function ptr, lambda or std::function)
    /// by checking if it has () operator
    /// </summary>
    template<typename T>
    concept IsInvocable = requires(T t) { &T::operator(); };

    template <typename TEnum>
    concept HasBitwiseAnd = requires(TEnum a, TEnum b) { { a& b } -> std::convertible_to<TEnum>; };
    template <typename TEnum>
    concept HasBitwiseOr = requires(TEnum a, TEnum b) { { a | b } -> std::convertible_to<TEnum>; };
    template <typename TEnum>
    concept HasBitwiseNot = requires(TEnum a) { { ~a } -> std::convertible_to<TEnum>; };

    template<typename T1, typename T2>
    concept IsLessComparable = requires(T1 t1, T2 t2) { { t1 < t2 } -> std::convertible_to<bool>; };
    template<typename T1, typename T2>
    concept IsGreaterComparable = requires(T1 t1, T2 t2) { { t1 > t2 } -> std::convertible_to<bool>; };
    template<typename T1, typename T2>
    concept IsEqualComparable = requires(T1 t1, T2 t2) { { t1 == t2 } -> std::convertible_to<bool>; };

    template <typename T, typename... Args>
    concept AllSameType = (std::same_as<T, Args> && ...);

    template<typename TBase, typename... Args>
    concept AllSameBaseType = (std::is_base_of_v<TBase, Args> && ...);

    template<typename... Args>
    concept HasAtLeastOneArg = sizeof...(Args) >= 1;

    template<size_t N, typename... Args>
    concept HasArgCount = sizeof...(Args) == N;

    template <typename T>
    struct ToPointerType { using Type = T*; };

    template <size_t Index, typename Variant>
    using VariantType = std::variant_alternative_t<Index, Variant>;

    //Variant holds has a member with type 'value_types'
    template <typename T>
    concept IsVariant = requires { typename T::value_types; };

    template <typename T>
    concept IsAllocatorType = requires 
    {
        typename T::value_type;
        requires std::derived_from<T, std::allocator<typename T::value_type>>;
    };

    template <typename T>
    concept IsComparableFunctorType =
        std::same_as<T, std::less<typename T::value_type>>
        || std::same_as<T, std::greater<typename T::value_type>>
        || std::same_as<T, std::less_equal<typename T::value_type>>
        || std::same_as<T, std::greater_equal<typename T::value_type>>;

    template <typename T>
    concept IsEqualFunctorType = requires
    {
        typename T::first_argument_type;
        typename T::second_argument_type;
        requires std::same_as<T, std::equal_to<typename T::first_argument_type>>;
    };

    template <typename T>
    concept IsHashType = requires
    {
        typename T::argument_type;
        typename T::result_type;
        requires std::same_as<T, std::hash<typename T::argument_type>>;
    };

    template <typename T>
    concept IsDataStructureMetaDataType = IsAllocatorType<T> || IsComparableFunctorType<T>
                                           || IsHashType<T> || IsEqualFunctorType<T>;

    /// <summary>
    /// True if T is a data structure with sequence like data such as:
    /// array, vector, queue, stack, deque, etc.
    /// </summary>
    template<typename T>
    concept IsSequenceLikeType = requires(T t) 
    {
        typename T::value_type;          
        { t.begin() } -> std::input_or_output_iterator;
        { t.end() } -> std::input_or_output_iterator;
    } && !requires { typename T::key_type; };

    /// <summary>
    /// True if T is a data structure with key value pairs such as:
    /// map, unordered map, multimap, unordered multimap
    /// </summary>
    template<typename T>
    concept IsMapLikeType = requires(T t) 
    {
        typename T::key_type;            
        typename T::mapped_type; 
        { t.find(std::declval<typename T::key_type>()) };
    };

    template<typename T>
    requires IsMapLikeType<T>
    struct MapTypeInfo 
    {
        using KeyType = typename T::key_type;
        using ValueType = typename T::mapped_type;
    };

    template<typename T>
    struct PairTypeInfo : std::false_type {};
    template<typename TKey, typename TValue>
    struct PairTypeInfo<std::pair<TKey, TValue>> : std::true_type
    {
        using KeyType = TKey;
        using ValueType = TValue;
    };
    template <typename T>
    concept IsPairType = (PairTypeInfo<std::remove_cv_t<T>>::value);
        
   /* template <typename T>
    struct ContainerTypeInfo {};

    template <
        template <typename...> class TContainer,
        typename TElement,
        typename... OtherArgs
    >
    struct ContainerTypeInfo<TContainer<TElement, OtherArgs...>> 
    {
        using ContainerTemplateType = TContainer;
        using ElementType = TElement;
    };*/

    template <typename T>
    struct ContainerTypeInfoHelper;
    template <template<typename...> class TContainer, typename... TArgs>
    struct ContainerTypeInfoHelper<TContainer<TArgs...>>
    {
        using ContainerTemplateType = TContainer<TArgs...>;
        using TypeParameterTuple = std::tuple<TArgs...>;
        using ElementType = std::tuple_element_t<0, TypeParameterTuple>;
    };
    template <typename T>
    struct ContainerTypeInfo : ContainerTypeInfoHelper<T> {};
}

