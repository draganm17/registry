#pragma once

#include <string>
#include <string_view>
#include <type_traits>


namespace registry {
namespace details {

    //------------------------------------------------------------------------------------//
    //                                  INTERFACE                                         //
    //------------------------------------------------------------------------------------//

    // Checks if 'T' is a string.
    // Derrives from 'std::true_type' if any of the following conditions is true:
    // - 'T' is a pointer to 'C', and 'is_character<C>::value == true';
    // - 'T' is an array of 'C', and 'is_character<C>::value == true';
    // - 'std::remove_cv_t<T>' is an std::basic_string<C, T, A>, and 'is_character<C>::value == true';
    // - 'std::remove_cv_t<T>' is an std::basic_string_view<C, T>, and 'is_character<C>::value == true'.
    // Otherwise, derrives from 'std::false_type'.
    template <typename T> struct is_string;

    // Checks if 'T' is a character.
    // Derrives from 'std::true_type' if any of the following conditions is true:
    // - 'std::remove_cv_t<T>' type is 'char';
    // - 'std::remove_cv_t<T>' type is 'signed char';
    // - 'std::remove_cv_t<T>' type is 'unsigned char';
    // - 'std::remove_cv_t<T>' type is 'wchar_t';
    // - 'std::remove_cv_t<T>' type is 'char16_t';
    // - 'std::remove_cv_t<T>' type is 'char32_t'.
    // Otherwise, derrives from 'std::false_type'.
    template <typename T> struct is_character;

    // A useful traits for strings.
    // This class has a specialization for each type 'T' for which 'is_string<T>::value == true'.
    template <typename T, typename = void>
    struct string_traits;
 // {
        // using char_type = /* string character type */

        // static size_t size(const T&)           { /* returns the string length */ }
        // static const char_type* data(const T&) { /* returns the string data */ }
 // };


    //------------------------------------------------------------------------------------//
    //                            IMPLEMENTATION DETAILS                                  //
    //------------------------------------------------------------------------------------//

    template <typename T> struct is_character
    : std::conditional_t<std::is_same<std::remove_cv_t<T>, char>::value          ||
                         std::is_same<std::remove_cv_t<T>, signed char>::value   ||
                         std::is_same<std::remove_cv_t<T>, unsigned char>::value ||
                         std::is_same<std::remove_cv_t<T>, wchar_t>::value       || 
                         std::is_same<std::remove_cv_t<T>, char16_t>::value      || 
                         std::is_same<std::remove_cv_t<T>, char32_t>::value,
                         std::true_type, std::false_type>
    { };


    template <typename T, typename = void>
    struct is_character_pointer : std::false_type { };

    template <typename T>
    struct is_character_pointer<T, typename std::enable_if_t<std::is_pointer<T>::value && 
                                                             is_character<std::remove_pointer_t<T>>::value>>
    : std::true_type { };

    template <typename T, typename = void>
    struct is_character_array : std::false_type { };

    template <typename T, size_t N>
    struct is_character_array<T[N], typename std::enable_if_t<is_character<T>::value>> : std::true_type { };

    template <typename T, typename = void>
    struct is_std_character_string_impl : std::false_type { };

    template <typename CharT, typename Traits, typename Alloc>
    struct is_std_character_string_impl<std::basic_string<CharT, Traits, Alloc>,
                                        typename std::enable_if_t<is_character<CharT>::value>>
    : std::true_type { };

    template <typename T>
    struct is_std_character_string : is_std_character_string_impl<std::remove_cv_t<T>> { };

    template <typename T, typename = void>
    struct is_std_character_string_view_impl : std::false_type { };

    template <typename CharT, typename Traits>
    struct is_std_character_string_view_impl<std::basic_string_view<CharT, Traits>,
                                             typename std::enable_if_t<is_character<CharT>::value>>
    : std::true_type { };

    template <typename T>
    struct is_std_character_string_view : is_std_character_string_view_impl<std::remove_cv_t<T>> { };

    template <typename T> struct is_string
    : std::conditional_t<is_character_pointer<T>::value         ||
                         is_character_array<T>::value           ||
                         is_std_character_string<T>::value      ||
                         is_std_character_string_view<T>::value,
                         std::true_type, std::false_type>
    { };

    template <typename T, typename = void>
    struct string_traits 
    {
        // TODO: rename char_type to value_type ???

        // using char_type = /* string character type */

        // static size_t size(const T&)           { /* returns the string length */ }
        // static const char_type* data(const T&) { /* returns the string data */ }
    };

    template <typename T>
    struct string_traits<T, typename std::enable_if_t<is_character_pointer<T>::value>>
    {
        using char_type = std::remove_cv_t<std::remove_pointer_t<T>>;

        static size_t size(const T& str) noexcept           { return std::char_traits<char_type>::length(str); }
        static const char_type* data(const T& str) noexcept { return str; }
    };

    template <typename T>
    struct string_traits<T, typename std::enable_if_t<is_character_array<T>::value>>
    {
        using char_type = std::remove_cv_t<std::remove_extent_t<T>>;

        static size_t size(const T& str) noexcept           { return sizeof(T) / sizeof(std::remove_extent_t<T>) - 1; }
        static const char_type* data(const T& str) noexcept { return str; }
    };

    template <typename T>
    struct string_traits<T, typename std::enable_if_t<is_std_character_string<T>::value || 
                                                      is_std_character_string_view<T>::value>>
    {
        using char_type = std::remove_cv_t<typename T::value_type>;

        static size_t size(const T& str) noexcept           { return str.size(); }
        static const char_type* data(const T& str) noexcept { return str.data(); }
    };

}} // namespace registry::details