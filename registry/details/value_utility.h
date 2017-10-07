#pragma once

#include <iterator>
#include <type_traits>

#include <registry/name.h>


namespace registry {
namespace details {

inline namespace range_traits
{
    using std::begin; using std::end;

    template <class T>
    using iterator_t = decltype(begin(std::declval<T&>()));

    template <class T>
    using iterator_category_t = typename std::iterator_traits<iterator_t<T>>::iterator_category;

    template <class T>
    using value_t = typename std::iterator_traits<iterator_t<T>>::value_type;

    template <typename T, typename = void>
    struct is_range_impl : std::false_type
    { };

    template <typename T>
    struct is_range_impl<T, std::void_t<decltype(begin(std::declval<T&>())),
                                        decltype(end(std::declval<T&>()))>>
    : std::true_type
    { };

    template <typename T>
    struct is_range : is_range_impl<T>
    { };

    template <typename T>
    struct is_input_range_impl 
    : std::conditional_t<std::is_same_v<iterator_category_t<T>, std::input_iterator_tag>,
                         std::true_type, std::false_type>
    { };

    template <typename T>
    struct is_input_range: std::conditional_t<is_range<T>::value,
                                              is_input_range_impl<T>, std::false_type>
    { };
}

    template <typename T> struct match_dword
    : std::conditional_t<sizeof(T) <= 4 && std::is_integral_v<T>, std::true_type, std::false_type>
    { };

    template <typename T> struct match_qword
    : std::conditional_t<sizeof(T) == 8 && std::is_integral_v<T>, std::true_type, std::false_type>
    { };

    template <typename T> struct match_sz
    : std::conditional_t<std::is_constructible_v<name, T>, std::true_type, std::false_type>
    { };

    template <typename T> struct match_multi_sz_impl
    : std::conditional_t<std::is_constructible_v<name, value_t<T>>, std::true_type, std::false_type>
    { };

    template <typename T> struct match_multi_sz
    : std::conditional_t<is_input_range<T>::value, match_multi_sz_impl<T>, std::false_type>
    { };

}} // namespace registry::details