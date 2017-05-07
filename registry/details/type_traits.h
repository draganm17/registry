#pragma once

#include <type_traits>

#include <registry/types.h>


namespace registry {

    class key_path;

namespace details {

    template <typename T, typename = void> static constexpr bool is_path_v = false;

    template <typename T> static constexpr bool is_path_v
        <T, typename std::enable_if_t<std::is_same<std::decay_t<T>, key_path>::value>> = true;

    template <typename T, typename = void> static constexpr bool is_pathable_v = false;

    template <typename T> static constexpr bool is_pathable_v
        <T, typename std::enable_if_t<std::is_constructible<key_path, T>::value>> = true;

    template <typename T, typename = void> static constexpr bool is_string_viewable_v = false;

    template <typename T> static constexpr bool is_string_viewable_v
        <T, typename std::enable_if_t<std::is_constructible<string_view_type, T>::value>> = true;

}} // namespace registry::details