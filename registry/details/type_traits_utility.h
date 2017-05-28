#pragma once

#include <type_traits>

#include <registry/types.h>


namespace registry {

    class key_path;

namespace details {

    template <typename T> struct is_path
    : std::conditional_t<std::is_same<std::remove_cv_t<T>, key_path>::value, std::true_type, std::false_type>
    { };

    template <typename T> struct is_pathable
    : std::conditional_t<std::is_constructible<key_path, T>::value, std::true_type, std::false_type>
    { };

}} // namespace registry::details