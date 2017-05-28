#pragma once

#include <string>
#include <string_view>
#include <type_traits>


namespace registry {
namespace details {
namespace encoding {

    //------------------------------------------------------------------------------------//
    //                                  INTERFACE                                         //
    //------------------------------------------------------------------------------------//

    // Checks whether 'T' is a character type.
    // Can be used as a customization point.
    // Enabled specializations should derrive from 'std::true_type'.
    // Specializations are already enabled for cv-qualified variants of those types:
    // 'char', 'signed char', 'unsigned char', 'wchar_t', 'char16_t' and 'char32_t'.
    template <typename T, typename Enable = void>
    struct is_character : std::false_type { };

    // Checks whether 'T' is a string type.
    // Can be used as a customization point.
    // Enabled specializations should derrive from 'std::true_type'.
    // Specializations are already enabled for:
    // - Character pointers (i.e. 'T' is a pointer to 'C', and 'is_character<C>::value == true');
    // - Character arrays (i.e. 'T' is an array of 'C', and 'is_character<C>::value == true');
    // - Standart strings (i.e. 'std::remove_cv_t<T>' is an 'std::basic_string<C, T, A>', and 'is_character<C>::value == true');
    // - Standart string views (i.e. 'std::remove_cv_t<T>' is an 'std::basic_string_view<C, T>', and 'is_character<C>::value == true').
    // NOTE: for each enabled specialization of 'is_string<T>' an specialization of 'string_traits<T>' should be provided.
    template <typename T, typename Enable = void>
    struct is_string : std::false_type { };

    // Determines the character type of 'T', which is a string or character type.
    // If 'is_character<T>::value' is true, the member typedef 'type' is 'std::remove_cv_t<T>'.
    // Otherwise, if 'is_string<T>::value' is true, the member typedef 'type' is 'string_traits<T>::char_type'.
    // Otherwise, the expression 'character_type<T>::type' is ill-formed.
    template <typename T, typename = void>
    struct character_type
    {
        // using type - /* the character type deduced from 'T' */
    };

    // Helper type
    template <typename T>
    using character_type_t = typename character_type<T>::type;

    // A useful traits for strings.
    // Enabled specializations should provide members as shown below.
    // NOTE: This class should have an enabled specialization for each
    //       type 'T' for which 'is_string<T>' specialization is enabled.
    template <typename T, typename Enable = void>
    struct string_traits
    {
        // using char_type = /* the character type of the string 'T' */

        // static size_t size(const T&)           { /* returns the string length */ }
        // static const char_type* data(const T&) { /* returns the string data */ }
    };


    //------------------------------------------------------------------------------------//
    //                            IMPLEMENTATION DETAILS                                  //
    //------------------------------------------------------------------------------------//

    template <typename T>
    struct is_character<T, std::enable_if_t<std::is_same<std::remove_cv_t<T>, char>::value          ||
                                            std::is_same<std::remove_cv_t<T>, signed char>::value   ||
                                            std::is_same<std::remove_cv_t<T>, unsigned char>::value ||
                                            std::is_same<std::remove_cv_t<T>, wchar_t>::value       ||
                                            std::is_same<std::remove_cv_t<T>, char16_t>::value      ||
                                            std::is_same<std::remove_cv_t<T>, char32_t>::value>>
    : std::true_type { };


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

    template <typename T>
    struct is_string<T, std::enable_if_t<is_character_pointer<T>::value    ||
                                         is_character_array<T>::value      ||
                                         is_std_character_string<T>::value ||
                                         is_std_character_string_view<T>::value>>
    : std::true_type { };

    template <typename T>
    struct character_type<T, std::enable_if_t<is_character<T>::value>>
    {
        using type = std::remove_cv_t<T>;
    };

    template <typename T>
    struct character_type<T, std::enable_if_t<is_string<T>::value>>
    {
        using type = typename string_traits<T>::char_type;
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

}}} // namespace registry::details::encoding