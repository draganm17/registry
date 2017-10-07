/** @file */
#pragma once

#include <algorithm>
#include <cstdint>
#include <locale>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <vector>

//#include <registry/details/encoding.h>
#include <registry/details/value_utility.h>
//#include <registry/types.h>  // TODO: ???


namespace registry
{
    //! The type of a registry value.
    /*! A registry value can store data in various formats. When you store data under a registry
    //  value, you can specify one of the following values to indicate the type of data being stored.
    //  For more information see: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724884
    */
    enum class value_type : uint32_t
    {
        /*! No defined value type. */
        none =                        0,

        /*! A null-terminated string. */
        sz =                          1,
        
        /*! A null-terminated string that contains unexpanded references to environment variables 
            (for example, "%PATH%"). */
        expand_sz =                   2,

        /*! Binary data in any form. */
        binary =                      3,
        
        /*! A 32-bit number. */
        dword =                       4,
        
        /*! A 32-bit number in big-endian format. */
        dword_big_endian =            5,
        
        /*! A null-terminated string that contains the target path of a symbolic link. */
        link =                        6,
        
        /*! A sequence of null-terminated strings, terminated by an empty string (\0). */
        multi_sz =                    7,
        
        /*! A 64-bit number. */
        qword =                       11
    };

    /*! \brief
    //  Defines an empty class type used to disambiguate the overloads of constructors and member
    //  functions of `registry::value`.
    */
    struct sz_value_tag               { };

    /*! \brief
    //  Defines an empty class type used to disambiguate the overloads of constructors and member
    //  functions of `registry::value`.
    */
    struct expand_sz_value_tag        { };

    /*! \brief
    //  Defines an empty class type used to disambiguate the overloads of constructors and member
    //  functions of `registry::value`.
    */
    struct dword_value_tag            { };

    /*! \brief
    //  Defines an empty class type used to disambiguate the overloads of constructors and member
    //  functions of `registry::value`.
    */
    struct dword_big_endian_value_tag { };

    /*! \brief
    //  Defines an empty class type used to disambiguate the overloads of constructors and member
    //  functions of `registry::value`.
    */
    struct link_value_tag             { };

    /*! \brief
    //  Defines an empty class type used to disambiguate the overloads of constructors and member
    //  functions of `registry::value`.
    */
    struct multi_sz_value_tag         { };

    /*! \brief
    //  Defines an empty class type used to disambiguate the overloads of constructors and member
    //  functions of `registry::value`.
    */
    struct qword_value_tag            { };


    //-------------------------------------------------------------------------------------------//
    //                                  class bad_value_cast                                     //
    //-------------------------------------------------------------------------------------------//

    //! Defines a type of object to be thrown by `registry::value` conversion functions on failure.
    class bad_value_cast : public std::bad_cast
    {
    public:
        const char* what() const noexcept override;
    };


    //-------------------------------------------------------------------------------------------//
    //                                       class value                                         //
    //-------------------------------------------------------------------------------------------//

    //! Represents the content of a registry value.
    /*! Objects of type `registry::value` represent a typed piece of data that can be written to
    //  or readed from the Windows registry by using the registry library API. Values are raw-data 
    //  storages that does not handle syntactic or semantic aspects of the data. However,
    //  `registry::value` provides convenient constructors to help users create values that are 
    //  suitable for correctly represent a registry value of a given type.
    */
    // TODO: rewrite the main description
    class value
    {
    private:
        value& do_assign(value_type type, const void* data, size_t size);

        value& do_assign(value_type type, std::basic_string_view<name::value_type> val);

        value& do_assign(value_type type, std::vector<std::wstring>&& val);

    public:
        //! Default constructor.
        /*!
        //  @post
        //    - `type() == value_type::none`.
        //    - `data() == nullptr`.
        //    - `size() == 0`.
        */
        value() noexcept = default;

        //! Constructs the value with the copy of the contents of `other`.
        /*!
        //  @post `*this == other`.
        */
        value(const value& other) = default;

        /*! \brief
        //  Constructs the value with the contents of `other` using move semantics. `other` is left
        //  in a valid but unspecified state. */
        /*!
        //  @post `*this` has the original value of `other`.
        */
        value(value&& other) noexcept = default;

        //! Constructs a value of type `value_type::none`.
        /*!
        //  @post
        //      - `type() == value_type::none`.
        //      - `data() == nullptr`.
        //      - `size() == 0`.
        */
        value(nullptr_t) noexcept;

        //! Constructs a value of type `value_type::dword` as if by `value(dword_value_tag(), val)`.
        /*!
        //  @post
        //      - `type() == value_type::dword`.
        //      - `to_uint32() == val`.
        //
        //  @param[in] val - an integer to be stored in this value.
        //
        //  @note This constructor only participates in overloading if:
        //        - `T` is an integral type;
        //        - `sizeof(T)` is less than or equal to 4.
        */
        template <typename T,
                  std::enable_if_t<details::match_dword<T>::value, T>* = 0
        >
        value(T val);

        //! Constructs a value of type `value_type::qword` as if by `value(qword_value_tag(), val)`.
        /*!
        //  @post
        //     - `type() == value_type::qword`.
        //     - `to_uint64() == val`.
        //
        //  @param[in] val - an integer to be stored in this value.
        //
        //  @note This constructor only participates in overloading if:
        //        - `T` is an integral type;
        //        - `sizeof(T)` is equal to 8.
        */
        template <typename T,
                  std::enable_if_t<details::match_qword<T>::value, T>* = 0
        >
        value(T val);

        //! Constructs a value of type `value_type::sz` as if by `value(sz_value_tag(), val)`.
        /*!
        //  @post
        //      - `type() == value_type::sz`.
        //      - `to_XXX() == val`. TODO
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @note This constructor only participates in overloading if `registry::name`
        //        is constructible from `T`.
        */
        template <typename T,
                  std::enable_if_t<details::match_sz<T>::value, T>* = 0
        >
        value(const T& val);

        //! Constructs a value of type `value_type::multi_sz` as if by `value(multi_sz_value_tag(), val)`.
        /*!
        //  @post `type() == value_type::multi_sz`.
        //
        //  @param[in] val - a collection of strings to be stored in this value.
        //
        //  @note This constructor only participates in overloading if:
        //        - `T` is an input range;
        //        - `registry::name` is constructible from `U`, where `U` is the value type of `T`.
        */
        template <typename T,
                  std::enable_if_t<details::match_multi_sz<T>::value, T>* = 0
        >
        value(const T& val);

        //! Constructs a value of type `value_type::sz`.
        /*!
        //  @post
        //      - `type() == value_type::sz`.
        //      - `to_XXX() == val`. TODO
        //
        //  @param[in] val - a string to be stored in this value.
        */
        value(sz_value_tag, const name& val);

        //! Constructs a value of type `value_type::sz`.
        /*!
        //  @post
        //      - `type() == value_type::sz`.
        //      - `to_XXX() == name(val, loc)`. TODO
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @param[in] loc - a locale that defines encoding conversion to use.
        */
        template <typename Source>
        value(sz_value_tag, const Source& val, const std::locale& loc = std::locale());

        //! Constructs a value of type `value_type::sz`.
        /*!
        //  @tparam InputIt - shall satisfy requirements of `InputIterator`.
        //
        //  @post
        //      - `type() == value_type::sz`.
        //      - `to_XXX() == name(first, last, loc)`. TODO
        //
        //  @param[in] first, last - a pair of iterators that specify a character sequence.
        //
        //  @param[in] loc         - a locale that defines encoding conversion to use.
        */
        template <typename InputIt>
        value(sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Constructs a value of type `value_type::sz`.
        /*!
        //  @post
        //      - `type() == value_type::expand_sz`.
        //      - `to_XXX() == val`. TODO
        //
        //  @param[in] val - a string to be stored in this value.
        */
        value(expand_sz_value_tag, const name& val);

        //! Constructs a value of type `value_type::expand_sz`.
        /*!
        //  @post
        //      - `type() == value_type::expand_sz`.
        //      - `to_XXX() == name(val, loc)`. TODO
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @param[in] loc - a locale that defines encoding conversion to use.
        */
        template <typename Source>
        value(expand_sz_value_tag, const Source& val, const std::locale& loc = std::locale());

        //! Constructs a value of type `value_type::expand_sz`.
        /*!
        //  @tparam InputIt - shall satisfy requirements of `InputIterator`.
        //
        //  @post
        //      - `type() == value_type::expand_sz`.
        //      - `to_XXX() == name(first, last, loc)`. TODO
        //
        //  @param[in] first, last - a pair of iterators that specify a character sequence.
        //
        //  @param[in] loc         - a locale that defines encoding conversion to use.
        */
        template <typename InputIt>
        value(expand_sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Constructs a value of type `value_type::dword`.
        /*!
        //  @post
        //    - `type() == value_type::dword`.
        //    - `to_uint32() == val`.
        //
        //  @param[in] val - an integer to be stored in this value.
        */
        value(dword_value_tag, uint32_t val);

        //! Constructs a value of type `value_type::dword_big_endian`.
        /*!
        //  @post
        //      - `type() == value_type::dword_big_endian`.
        //      - `to_uint32() == val`.
        //
        //  @param[in] val - an integer to be stored in this value.
        */
        value(dword_big_endian_value_tag, uint32_t val);

        //! Constructs a value of type `value_type::sz`.
        /*!
        //  @post
        //      - `type() == value_type::link`.
        //      - `to_XXX() == val`. TODO
        //
        //  @param[in] val - a string to be stored in this value.
        */
        value(link_value_tag, const name& val);

        //! Constructs a value of type `value_type::link`.
        /*!
        //  @post
        //      - `type() == value_type::link`.
        //      - `to_XXX() == name(val, loc)`. TODO
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @param[in] loc - a locale that defines encoding conversion to use.
        */
        template <typename Source>
        value(link_value_tag, const Source& val, const std::locale& loc = std::locale());

        //! Constructs a value of type `value_type::link`.
        /*!
        //  @tparam InputIt - shall satisfy requirements of `InputIterator`.
        //
        //  @post
        //      - `type() == value_type::link`.
        //      - `to_XXX() == name(first, last, loc)`. TODO
        //
        //  @param[in] first, last - a pair of iterators that specify a character sequence.
        //
        //  @param[in] loc         - a locale that defines encoding conversion to use.
        */
        template <typename InputIt>
        value(link_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Constructs a value of type `value_type::multi_sz`.
        /*!
        //  @tparam Source - shall satisfy the requirements of `InputRange`. `registry::name`
        //                   shall be constructible from `U`, where `U` is the value type of `Source`.
        //
        //  @post `type() == value_type::multi_sz`. TODO: ???
        //
        //  @param[in] val - a collection of strings to be stored in this value.
        */
        template <typename Source>
        value(multi_sz_value_tag, const Source& val, const std::locale& loc = std::locale());

        //! Constructs a value of type `value_type::multi_sz`.
        /*!
        //  @tparam InputIt - TODO: ...
        //
        //  @post `type() == value_type::multi_sz`.
        //
        //  @param[in] first, last - input iterators, such as `std::iterator_traits<InputIt>::value_type`
        //                           should be explicitly convertible to `registry::string_view_type`.
        //
        //  @param[in] loc         - TODO: ...
        */
        // TODO: rewrite description
        template <typename InputIt>
        value(multi_sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Constructs a value of type `value_type::qword`.
        /*!
        //  @post
        //      - `type() == value_type::qword`.
        //      - `to_uint64() == val`.
        //
        //  @param[in] val - an integer to be stored in this value.
        */
        value(qword_value_tag, uint64_t val);

        //! Constructs the value from a value type identifier and binary data.
        /*!
        //  Any byte sequence is legal, the format of the data is not checked over the value type.
        //  However, if the stored byte sequence is not suitable for representing a value of a given
        //  type, then calling a conversion function may produce a valid but undefined result. \n
        //  If the value type is one of `value_type::sz`, `value_type::expand_sz`, `value_type::link` 
        //  or `value_type::multi_sz`, providing the null terminator character is desirable but not
        //  necessary.
        //
        //  @post
        //      - `type() == type`.
        //      - `size() == size`.
        //      - `memcmp(data, data(), size()) == 0`.
        //
        //  @param[in] type - a value type identifier.
        //
        //  @param[in] data - the binary data to be stored in this value.
        //
        //  @param[in] size - the size of the binary data in bytes.
        */
        value(value_type type, const void* data, size_t size);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        //  @post `*this == other`.
        //
        //  @return `*this`.
        */
        value& operator=(const value& other) = default;

        /*! \brief
            Replaces the contents of `*this` with those of `other` using move semantics. `other` is 
            left in a valid, but unspecified state. */
        /*!
        //  @post `*this` has the original value of `other`.
        //
        //  @return `*this`.
        */
        value& operator=(value&& other) noexcept = default;

        //! Replaces the contents to the value as if by `assign(nullptr)`.
        /*!
        //  @post `*this == value(nullptr)`.
        //
        //  @return `*this`.
        */
        value& operator=(nullptr_t);

        //! Replaces the contents to the value as if by `assign(dword_value_tag(), val)`.
        /*!
        //  @post `*this == value(val)`.
        //
        //  @param[in] val - an integer to be stored in this value.
        //
        //  @note This operator only participates in overloading if:
        //        - `T` is an integral type;
        //        - `sizeof(T)` is less than or equal to 4.
        //
        //  @return `*this`.
        */
        template <typename T,
                  std::enable_if_t<details::match_dword<T>::value, T>* = 0
        >
        value& operator=(T val);

        //! Replaces the contents to the value as if by `assign(qword_value_tag(), val)`.
        /*!
        //  @post `*this == value(val)`.
        //
        //  @param[in] val - an integer to be stored in this value.
        //
        //  @note This operator only participates in overloading if:
        //        - `T` is an integral type;
        //        - `sizeof(T)` is equal to 8.
        //
        //  @return `*this`.
        */
        template <typename T,
                  std::enable_if_t<details::match_qword<T>::value, T>* = 0
        >
        value& operator=(T val);

        //! Replaces the contents to the value as if by `assign(sz_value_tag(), val)`.
        /*!
        //  @post `*this == value(val)`.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @note This constructor only participates in overloading if `registry::name`
        //        is constructible from `T`.
        //
        //  @return `*this`.
        */
        template <typename T,
                  std::enable_if_t<details::match_sz<T>::value, T>* = 0
        >
        value& operator=(const T& val);

        //! Replaces the contents to the value as if by `assign(multi_sz_value_tag(), val)`.
        /*!
        //  @post `*this == value(val)`.
        //
        //  @param[in] val - a collection of strings to be stored in this value.
        //
        //  @note This constructor only participates in overloading if:
        //        - `T` is an input range;
        //        - `registry::name` is constructible from `U`, where `U` is the value type of `T`.
        //
        //  @return `*this`.
        */
        template <typename T,
                  std::enable_if_t<details::match_multi_sz<T>::value, T>* = 0
        >
        value& operator=(const T& val);

    public:
        //! Returns the value type.
        value_type type() const noexcept;

        //! Returns the value data.
        const void* data() const noexcept;

        //! Returns the value size.
        size_t size() const noexcept;

    public:
        //! Converts the value to an unsigned 32-bit integer.
        /*!
        //  @throw `registry::bad_value_cast` if the value type is not one of `value_type::dword` 
                   or `value_type::dword_big_endian`.
        */
        uint32_t to_uint32() const;

        //! Converts the value to an unsigned 64-bit integer.
        /*!
        //  @throw `registry::bad_value_cast` if the value type is not one of `value_type::dword`, 
                   `value_type::dword_big_endian` or `value_type::qword`.
        */
        uint64_t to_uint64() const;

        //! Converts the value to a string.
        /*!
        //  @throw `registry::bad_value_cast` if the value type is not one of `value_type::sz`,
                   `value_type::expand_sz` or `value_type::link`.
        */
        // TODO: rewrite description
        //       ad the descr. of all members that reffer to 'to_string()'
        std::string to_string(const std::locale& loc = std::locale()) const;

        //! TODO: ...
        std::wstring to_wstring() const;

        //! Converts the value to an array of strings.
        /*!
        //  @throw `registry::bad_value_cast` if the value type is not `value_type::multi_sz`.
        */
        std::vector<std::string> to_strings(const std::locale& loc = std::locale()) const;

        //! TODO: ...
        std::vector<std::wstring> to_wstrings() const;

    public:
        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(nullptr)`.
        //
        //  @return `*this`.
        */
        value& assign(nullptr_t);

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(sz_value_tag(), val)`.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(sz_value_tag, const name& val);

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(sz_value_tag(), val, loc)`.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @return `*this`.
        */
        template <typename Source>
        value& assign(sz_value_tag, const Source& val, const std::locale& loc = std::locale());

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(sz_value_tag(), first, last, loc)`.
        //
        //  @param[in] first, last - a pair of iterators that specify a character sequence.
        //
        //  @param[in] loc         - a locale that defines encoding conversion to use.
        //
        //  @return `*this`.
        */
        template <typename InputIt>
        value& assign(sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(expand_sz_value_tag(), val)`.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(expand_sz_value_tag, const name& val);

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(expand_sz_value_tag(), val, loc)`.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @return `*this`.
        */
        template <typename Source>
        value& assign(expand_sz_value_tag, const Source& val, const std::locale& loc = std::locale());

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(expand_sz_value_tag(), first, last, loc)`.
        //
        //  @param[in] first, last - a pair of iterators that specify a character sequence.
        //
        //  @param[in] loc         - a locale that defines encoding conversion to use.
        //
        //  @return `*this`.
        */
        template <typename InputIt>
        value& assign(expand_sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(dword_value_tag(), val)`.
        //
        //  @param[in] val - an integer to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(dword_value_tag, uint32_t val);

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(dword_big_endian_value_tag(), val)`.
        //
        //  @param[in] val - an integer to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(dword_big_endian_value_tag, uint32_t val);

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(link_value_tag(), val)`.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(link_value_tag, const name& val);

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(link_value_tag(), val, loc)`.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @return `*this`.
        */
        template <typename Source>
        value& assign(link_value_tag, const Source& val, const std::locale& loc = std::locale());

        //! Replaces the contents of the value.
        /*!
        //  @post `*this == value(link_value_tag(), first, last, loc)`.
        //
        //  @param[in] first, last - a pair of iterators that specify a character sequence.
        //
        //  @param[in] loc         - a locale that defines encoding conversion to use.
        //
        //  @return `*this`.
        */
        template <typename InputIt>
        value& assign(link_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Replaces the contents of the value.
        /*!
        //  @tparam Source - shall satisfy the requirements of `InputRange`. `registry::name`
        //                   shall be constructible from `U`, where `U` is the value type of `Source`.
        //
        //  @post `type() == value_type::multi_sz`. TODO: ???
        //
        //  @param[in] val - a collection of strings to be stored in this value.
        */
        template <typename Sequence>
        value& assign(multi_sz_value_tag, const Sequence& val, const std::locale& loc = std::locale());

        //! Replaces the contents of the value.
        /*!
        //  @tparam InputIt - TODO: ...
        //
        //  @param[in] first, last - input iterators, such as `std::iterator_traits<InputIt>::value_type` 
        //                           should be explicitly convertible to `registry::string_view_type`.
        //
        //  @param[in] loc         - TODO: ...
        //
        //  @return `*this`.
        */
        // TODO: rewrite description
        template <typename InputIt>
        value& assign(multi_sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! TODO: ...
        /*!
        //  @post `*this == value(qword_value_tag(), val)`.
        //
        //  @param[in] val - an unsigned 64-bit integer to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(qword_value_tag, uint64_t val);

        //! TODO: ...
        /*!
        //  @post `*this == value(type, data, size)`.
        //
        //  @param[in] type - a value type identifier.
        //
        //  @param[in] data - the binary data to be stored in this value.
        //
        //  @param[in] size - the size of the binary data in bytes.
        //
        //  @return `*this`.
        */
        value& assign(value_type type, const void* data, size_t size);

        //! Swaps the contents of `*this` and `other`.
        void swap(value& other) noexcept;

    private:
        value_type               m_type = value_type::none;
      
        std::basic_string<char>  m_data;
        // NOTE: using std::basic_string as a container allows small data optimization.
    };


    //-------------------------------------------------------------------------------------------//
    //                                   NON-MEMBER FUNCTIONS                                    //
    //-------------------------------------------------------------------------------------------//

    //! Checks whether `lhs` is equal to `rhs`.
    bool operator==(const value& lhs, const value& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`.
    bool operator!=(const value& lhs, const value& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`.
    bool operator<(const value& lhs, const value& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`.
    bool operator>(const value& lhs, const value& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`.
    bool operator<=(const value& lhs, const value& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`.
    bool operator>=(const value& lhs, const value& rhs) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(value& lhs, value& rhs) noexcept;


    //-------------------------------------------------------------------------------------------//
    //                                    INLINE DEFINITIONS                                     //
    //-------------------------------------------------------------------------------------------//

    inline const char* bad_value_cast::what() const noexcept
    {
        return "registry::bad_value_cast";
    }

    template <typename T,
              std::enable_if_t<details::match_dword<T>::value, T>* = 0
    >
    inline value::value(T val)
    : value(dword_value_tag(), val)
    { }

    template <typename T,
              std::enable_if_t<details::match_qword<T>::value, T>* = 0
    >
    inline value::value(T val)
    : value(qword_value_tag(), val)
    { }

    template <typename T,
              std::enable_if_t<details::match_sz<T>::value, T>* = 0
    >
    inline value::value(const T& val)
    : value(sz_value_tag(), val)
    { }

    template <typename T,
              std::enable_if_t<details::match_multi_sz<T>::value, T>* = 0
    >
    inline value::value(const T& val)
    : value(multi_sz_value_tag(), val)
    { }

    template <typename Source>
    inline value::value(sz_value_tag, const Source& val, const std::locale& loc)
    {
        assign(sz_value_tag(), val, loc);
    }

    template <typename InputIt>
    inline value::value(sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        assign(sz_value_tag(), first, last, loc);
    }

    template <typename Source>
    inline value::value(expand_sz_value_tag, const Source& val, const std::locale& loc)
    {
        assign(expand_sz_value_tag(), val, loc);
    }

    template <typename InputIt>
    inline value::value(expand_sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        assign(expand_sz_value_tag(), first, last, loc);
    }

    template <typename Source>
    inline value::value(link_value_tag, const Source& val, const std::locale& loc)
    {
        assign(link_value_tag(), val, loc);
    }

    template <typename InputIt>
    inline value::value(link_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        assign(link_value_tag(), first, last, loc);
    }

    template <typename Sequence>
    inline value::value(multi_sz_value_tag, const Sequence& val, const std::locale& loc)
    {
        assign(multi_sz_value_tag(), val, loc);
    }

    template <typename InputIt>
    inline value::value(multi_sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        assign(multi_sz_value_tag(), first, last, loc);
    }

    template <typename T,
              std::enable_if_t<details::match_dword<T>::value, T>* = 0
    >
    inline value& value::operator=(T val)
    {
        return assign(dword_value_tag(), val);
    }

    template <typename T,
              std::enable_if_t<details::match_qword<T>::value, T>* = 0
    >
    inline value& value::operator=(T val)
    {
        return assign(qword_value_tag(), val);
    }

    template <typename T,
              std::enable_if_t<details::match_sz<T>::value, T>* = 0
    >
    inline value& value::operator=(const T& val)
    {
        return assign(sz_value_tag(), val);
    }

    template <typename T,
              std::enable_if_t<details::match_multi_sz<T>::value, T>* = 0
    >
    inline value& value::operator=(const T& val)
    {
        return assign(multi_sz_value_tag(), val);
    }

    template <typename Source>
    inline value& value::assign(sz_value_tag, const Source& val, const std::locale& loc)
    {
        if constexpr(std::is_convertible_v<std::decay_t<Source>,
                                           std::basic_string_view<name::value_type>>)
        {
            return do_assign(value_type::sz, val);
        } else {
            return do_assign(value_type::sz, name(val, loc));
        }
    }

    template <typename InputIt>
    inline value& value::assign(sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        if constexpr(std::is_pointer_v<InputIt> &&
                     std::is_same_v<name::value_type, std::remove_cv_t<std::remove_pointer_t<InputIt>>>)
        {
            return do_assign(value_type::sz, { first, last - first });
        } else {
            return do_assign(value_type::sz, name(first, last, loc));
        }
    }

    template <typename Source>
    inline value& value::assign(expand_sz_value_tag, const Source& val, const std::locale& loc)
    {
        if constexpr(std::is_convertible_v<std::decay_t<Source>,
                                           std::basic_string_view<name::value_type>>)
        {
            return do_assign(value_type::expand_sz, val);
        } else {
            return do_assign(value_type::expand_sz, name(val, loc));
        }
    }

    template <typename InputIt>
    inline value& value::assign(expand_sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        if constexpr(std::is_pointer_v<InputIt> &&
                     std::is_same_v<name::value_type, std::remove_cv_t<std::remove_pointer_t<InputIt>>>)
        {
            return do_assign(value_type::expand_sz, { first, last - first });
        } else {
            return do_assign(value_type::expand_sz, name(first, last, loc));
        }
    }

    template <typename Source>
    inline value& value::assign(link_value_tag, const Source& val, const std::locale& loc)
    {
        if constexpr(std::is_convertible_v<std::decay_t<Source>,
                                           std::basic_string_view<name::value_type>>)
        {
            return do_assign(value_type::link, val);
        } else {
            return do_assign(value_type::link, name(val, loc));
        }
    }

    template <typename InputIt>
    inline value& value::assign(link_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        if constexpr(std::is_pointer_v<InputIt> &&
                     std::is_same_v<name::value_type, std::remove_cv_t<std::remove_pointer_t<InputIt>>>)
        {
            return do_assign(value_type::link, { first, last - first });
        } else {
            return do_assign(value_type::link, name(first, last, loc));
        }
    }

    template <typename Sequence>
    inline value& value::assign(multi_sz_value_tag, const Sequence& val, const std::locale& loc)
    {
        std::vector<name> names;
        using std::begin; using std::end;
        std::transform(begin(val), end(val), std::back_inserter(names), [&](auto&& el)
                                                                        { return name(el, loc); });

        return do_assign(value_type::multi_sz, std::move(names));
    }

    template <typename InputIt>
    inline value& value::assign(multi_sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        std::vector<name> names;
        std::transform(first, last, std::back_inserter(names), [&](auto&& el)
                                                               { return name(el, loc); });

        return do_assign(value_type::multi_sz, std::move(names));
    }

    inline bool operator==(const value& lhs, const value& rhs) noexcept
    {
        return lhs.type() == rhs.type() && lhs.data() == rhs.data();
    }

    inline bool operator!=(const value& lhs, const value& rhs) noexcept
    {
        return !(lhs == rhs);
    }

    inline bool operator<(const value& lhs, const value& rhs) noexcept
    {
        return lhs.type() < rhs.type() || (lhs.type() == rhs.type() && lhs.data() < rhs.data());
    }

    inline bool operator>(const value& lhs, const value& rhs) noexcept
    {
        return lhs.type() > rhs.type() || (lhs.type() == rhs.type() && lhs.data() > rhs.data());
    }

    inline bool operator<=(const value& lhs, const value& rhs) noexcept
    {
        return !(lhs > rhs);
    }

    inline bool operator>=(const value& lhs, const value& rhs) noexcept
    {
        return !(lhs < rhs);
    }

    inline void swap(value& lhs, value& rhs) noexcept
    {
        lhs.swap(rhs);
    }

} // namespace registry


namespace std
{
    //-------------------------------------------------------------------------------------------//
    //                               class hash<registry::value>                                 //
    //-------------------------------------------------------------------------------------------//

    //! std::hash specialization for `registry::value`.
    template <>
    struct hash<registry::value>
    {
        //! Calculates a hash value for a `value` object.
        /*!
        //  @return A hash value such that if for two values, `v1 == v2`
        //          then `hash<registry::value>()(v1) == hash<registry::value>()(v2)`.
        */
        size_t operator()(const registry::value& val) const noexcept;
    };

} // namespace std