/** @file */
#pragma once

#include <cstdint>
#include <initializer_list>
#include <typeinfo>
#include <type_traits>
#include <vector>

#include <registry/types.h>


namespace registry
{
    /*! A registry value can store data in various formats. When you store data under a registry value, you can
    specify one of the following values to indicate the type of data being stored. For more information see: 
    https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724884 */
    enum class value_type : uint32_t
    {
        /*! No defined value type. */
        none =                 0,

        /*! A null-terminated string. */
        sz =                   1,
        
        /*! A null-terminated string that contains unexpanded references to environment variables 
        (for example, "%PATH%"). */
        expand_sz =            2,

        /*! Binary data in any form. */
        binary =               3,
        
        /*! A 32-bit number. */
        dword =                4,
        
        /*! A 32-bit number in big-endian format. */
        dword_big_endian =     5,
        
        /*! A null-terminated string that contains the target path of a symbolic link. */
        link =                 6,
        
        /*! A sequence of null-terminated strings, terminated by an empty string (\0). */
        multi_sz =             7,
        
        /*! A 64-bit number. */
        qword =                11
    };
    //TODO: Should I support REG_RESOURCE_LIST, REG_FULL_RESOURCE_DESCRIPTOR and REG_RESOURCE_REQUIREMENTS_LIST types ?
    //      If not, should I specify an 'unknown' type in case such a value is readed into registry::value object ?

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct none_value_tag             { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct sz_value_tag               { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct expand_sz_value_tag        { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct binary_value_tag           { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct dword_value_tag            { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct dword_big_endian_value_tag { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct link_value_tag             { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct multi_sz_value_tag         { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or `assign` function.
    struct qword_value_tag            { };

    //------------------------------------------------------------------------------------//
    //                             class bad_value_cast                                   //
    //------------------------------------------------------------------------------------//

    //! Defines a type of object to be thrown by `registry::value` conversion functions on failure.
    class bad_value_cast : public std::bad_cast
    {
    public:
        const char* what() const noexcept override { return "registry::bad_value_cast"; }
    };

    //\cond HIDDEN_SYMBOLS
    namespace details
    {
        // TODO: get rid of value_state ???
        struct value_state
        {
            value_type       m_type = value_type::none;
            byte_array_type  m_data;
        };
    } //\endcond

    //------------------------------------------------------------------------------------//
    //                                 class value                                        //
    //------------------------------------------------------------------------------------//

    //! Represents a registry value.
    /*!
    Objects of type `registry::value` represent a typed piece of data that can be written to or readed from the
    Windows registry by using the registry library API. Values are raw-data storages that does not handle syntactic
    or semantic aspects of the data. However, `registry::value` provides convenient constructors to help users create
    values that are suitable for correctly represent a registry value of a given type.
    */
    class value 
        : private details::value_state
    {
    public:
        //! Default constructor.
        /*!
        @post `type() == value_type::none`.
        @post `data().empty()`.
        */
        value() noexcept = default;

        //! Constructs the value with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        value(const value& other) = default;

        /*! \brief
        Constructs the value with the contents of `other` using move semantics. `other` is left in a valid but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        value(value&& other) noexcept = default;

        //! Constructs a value of type `value_type::none`.
        /*!
        @post `type() == value_type::none`.
        @post `data().empty()`.
        @param[in] tag - value type tag.
        */
        explicit value(none_value_tag tag) noexcept;

        //! Constructs a value of type `value_type::sz`.
        /*!
        @post `type() == value_type::sz`.
        @post `to_string() == value`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        */
        value(sz_value_tag tag, string_view_type value);

        //! Constructs a value of type `value_type::expand_sz`.
        /*!
        @post `type() == value_type::expand_sz`.
        @post `to_string() == value`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        */
        value(expand_sz_value_tag tag, string_view_type value);

        //! Constructs a value of type `value_type::binary`.
        /*!
        @post `type() == value_type::binary`.
        @post `to_byte_array() == value`.
        @param[in] tag - value type tag.
        @param[in] value - binary data to be stored in this value.
        */
        // TODO: replace 'byte_array_view_type' by 'const uint8_t*' and 'size_t' ???
        value(binary_value_tag tag, byte_array_view_type value);

        //! Constructs a value of type `value_type::dword`.
        /*!
        @post `type() == value_type::dword`.
        @post `to_uint32() == value && to_uint64() == value`.
        @param[in] tag - value type tag.
        @param[in] value - an unsigned 32-bit integer to be stored in this value.
        */
        value(dword_value_tag tag, uint32_t value);

        //! Constructs a value of type `value_type::dword_big_endian`.
        /*!
        @post `type() == value_type::dword_big_endian`.
        @post `to_uint32() == value && to_uint64() == value`.
        @param[in] tag - value type tag.
        @param[in] value - an unsigned 32-bit integer to be stored in this value.
        */
        value(dword_big_endian_value_tag tag, uint32_t value);

        //! Constructs a value of type `value_type::link`.
        /*!
        @post `type() == value_type::link`.
        @post `to_string() == value`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        */
        value(link_value_tag tag, string_view_type value);

        //! Constructs a value of type `value_type::multi_sz`.
        /*!
        @post `type() == value_type::multi_sz`.
        @post Let `seq = to_strings()`, then `std::equal(seq.begin(), seq.end(), BEGIN(value), END(value))`.
        @param[in] tag - value type tag.
        @param[in] value - a container, such as `Sequence::value_type` should be explicitly convertible to 
                           `registry::string_view_type`.
        */
        template <typename Sequence,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, Sequence::value_type>::value>
        >
        value(multi_sz_value_tag tag, const Sequence& value);

        //! Constructs a value of type `value_type::multi_sz`.
        /*!
        @post `type() == value_type::multi_sz`.
        @post Let `seq = to_strings()`, then `std::equal(seq.begin(), seq.end(), first, last)`.
        @param[in] tag - value type tag.
        @param[in] first, last - input iterators, such as `std::iterator_traits<InputIt>::value_type` should be 
                                 explicitly convertible to `registry::string_view_type`.
        */
        template <typename InputIt,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, 
                                                                    std::iterator_traits<InputIt>::value_type>::value>
        >
        value(multi_sz_value_tag tag, InputIt first, InputIt last);

        //! Constructs a value of type `value_type::multi_sz`.
        /*!
        @post `type() == value_type::multi_sz`.
        @post Let `seq = to_strings()`, then `std::equal(seq.begin(), seq.end(), init.begin(), init.end())`.
        @param[in] tag - value type tag.
        @param[in] init - an object of type `std::initializer_list<T>`, such as `T` should be explicitly convertible
                          to `registry::string_view_type`.
        */
        template <typename T,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, T>::value>
        >
        value(multi_sz_value_tag tag, std::initializer_list<T> init);

        //! Constructs a value of type `value_type::qword`.
        /*!
        @post `type() == value_type::qword`.
        @post `to_uint64() == value`.
        @param[in] tag - value type tag.
        @param[in] value - an unsigned 64-bit integer to be stored in this value.
        */
        value(qword_value_tag tag, uint64_t value);

        //! Constructs the value from a value type identifier and binary data.
        /*!
        Any byte sequence is legal, the format of the data is not checked over the value type. However, if the stored
        byte sequence is not suitable for representing a value of a given type, then calling a conversion function may
        produce a valid but undefined result. \n
        If the value type is one of value_type::sz, value_type::expand_sz, value_type::link or value_type::multi_sz, 
        providing the null terminator character is desirable but not necessary.
        @post `this->type() == type`.
        @post `this->data() == data`.
        @param[in] type - a value type identifier.
        @param[in] data - the binary data to be stored in this value.
        */
        value(value_type type, byte_array_view_type data);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        value& operator=(const value& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        value& operator=(value&& other) noexcept = default;

    public:
        //! Returns the value type.
        value_type type() const noexcept;

        //! Returns stored binary data if any.
        byte_array_view_type data() const noexcept;

        //! Converts the value to an unsigned 32-bit integer.
        /*!
        @throw `registry::bad_value_cast` if the value type is not one of `value_type::dword` or 
               `value_type::dword_big_endian`.
        */
        uint32_t to_uint32() const;

        //! Converts the value to an unsigned 64-bit integer.
        /*!
        @throw `registry::bad_value_cast` if the value type is not one of `value_type::dword`, 
               `value_type::dword_big_endian` or `value_type::qword`.
        */
        uint64_t to_uint64() const;

        //! Converts the value to a string.
        /*!
        @throw `registry::bad_value_cast` if the value type is not one of `value_type::sz`, `value_type::expand_sz`
               or `value_type::link`.
        */
        string_type to_string() const;

        //! Converts the value to an array of strings.
        /*!
        @throw `registry::bad_value_cast` if the value type is not `value_type::multi_sz`.
        */
        std::vector<string_type> to_strings() const;

        //! Converts the value to a binary data array.
        /*!
        @throw `registry::bad_value_cast` if the value type is not `value_type::binary`.
        */
        byte_array_type to_byte_array() const;

    public:
        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag)`.
        @param[in] tag - value type tag.
        @return `*this`.
        */
        value& assign(none_value_tag tag) noexcept;

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        @return `*this`.
        */
        value& assign(sz_value_tag tag, string_view_type value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        @return `*this`.
        */
        value& assign(expand_sz_value_tag tag, string_view_type value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] tag - value type tag.
        @param[in] value - binary data to be stored in this value.
        @return `*this`.
        */
        value& assign(binary_value_tag tag, byte_array_view_type value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] tag - value type tag.
        @param[in] value - an unsigned 32-bit integer to be stored in this value.
        @return `*this`.
        */
        value& assign(dword_value_tag tag, uint32_t value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] tag - value type tag.
        @param[in] value - an unsigned 32-bit integer to be stored in this value.
        @return `*this`.
        */
        value& assign(dword_big_endian_value_tag tag, uint32_t value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        @return `*this`.
        */
        value& assign(link_value_tag tag, string_view_type value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] tag - value type tag.
        @param[in] value - a container, such as `Sequence::value_type` should be explicitly convertible to 
                           `registry::string_view_type`.
        @return `*this`.
        */
        template <typename Sequence,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, Sequence::value_type>::value>
        >
        value& assign(multi_sz_value_tag, const Sequence& value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, first, last)`.
        @param[in] tag - value type tag.
        @param[in] first, last - input iterators, such as `std::iterator_traits<InputIt>::value_type` should be 
                                 explicitly convertible to `registry::string_view_type`.
        @return `*this`.
        */
        template <typename InputIt,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, 
                                                                    std::iterator_traits<InputIt>::value_type>::value>
        >
        value& assign(multi_sz_value_tag, InputIt first, InputIt last);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, init)`.
        @param[in] tag - value type tag.
        @param[in] init - an object of type `std::initializer_list<T>`, such as `T` should be explicitly convertible
                          to `registry::string_view_type`.
        @return `*this`.
        */
        template <typename T,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, T>::value>
        >
        value& assign(multi_sz_value_tag tag, std::initializer_list<T> init);

        // TODO: ...
        value& assign(multi_sz_value_tag, const std::vector<string_view_type>& value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] tag - value type tag.
        @param[in] value - an unsigned 64-bit integer to be stored in this value.
        @return `*this`.
        */
        value& assign(qword_value_tag, uint64_t value);

        //! Replaces the contents of the value.
        /*!
        @post `*this == value(tag, value)`.
        @param[in] type - a value type identifier.
        @param[in] data - the binary data to be stored in this value.
        @return `*this`.
        */
        value& assign(value_type type, byte_array_view_type data);

        //! Swaps the contents of `*this` and `other`.
        void swap(value& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

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

    //! Calculates a hash value for a `value` object.
    /*!
    @return A hash value such that if for two values, `v1 == v2` then `hash_value(v1) == hash_value(v2)`.
    */
    size_t hash_value(const value& value) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(value& lhs, value& rhs) noexcept;

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

    template <typename Sequence,
              typename = std::enable_if_t<std::is_constructible<string_view_type, Sequence::value_type>::value>
    >
    inline value::value(multi_sz_value_tag tag, const Sequence& value) { assign(tag, value); }

    template <typename InputIt,
              typename = std::enable_if_t<std::is_constructible<string_view_type, 
                                                                std::iterator_traits<InputIt>::value_type>::value>
    >
    inline value::value(multi_sz_value_tag tag, InputIt first, InputIt last) { assign(tag, first, last); }

    template <typename T,
              typename = std::enable_if_t<std::is_constructible<string_view_type, T>::value>
    >
    inline value::value(multi_sz_value_tag tag, std::initializer_list<T> init) { assign(tag, init); }

    template <typename Sequence,
              typename = std::enable_if_t<std::is_constructible<string_view_type, Sequence::value_type>::value>
    >
    inline value& value::assign(multi_sz_value_tag tag, const Sequence& value)
    {
        using std::begin; using std::end;
        return assign(tag, begin(value), end(value));
    }

    template <typename InputIt,
              typename = std::enable_if_t<std::is_constructible<string_view_type, 
                                                                std::iterator_traits<InputIt>::value_type>::value>
    >
    inline value& value::assign(multi_sz_value_tag tag, InputIt first, InputIt last)
    {
        std::vector<string_view_type> strings;
        std::transform(first, last, std::back_inserter(strings),
                       [](const auto& el) { return string_view_type(el); });
        return assign(tag, strings);
    }

    template <typename T,
              typename = std::enable_if_t<std::is_constructible<string_view_type, T>::value>
    >
    inline value& value::assign(multi_sz_value_tag tag, std::initializer_list<T> init) 
    { return assign(tag, init.begin(), init.end()); }

    inline bool operator==(const value& lhs, const value& rhs) noexcept 
    { return lhs.type() == rhs.type() && lhs.data() == rhs.data(); }

    inline bool operator!=(const value& lhs, const value& rhs) noexcept { return !(lhs == rhs); }

    inline bool operator<(const value& lhs, const value& rhs) noexcept 
    { return lhs.type() < rhs.type() || (lhs.type() == rhs.type() && lhs.data() < rhs.data()); }

    inline bool operator>(const value& lhs, const value& rhs) noexcept 
    { return lhs.type() > rhs.type() || (lhs.type() == rhs.type() && lhs.data() > rhs.data()); }

    inline bool operator<=(const value& lhs, const value& rhs) noexcept { return !(lhs > rhs); }

    inline bool operator>=(const value& lhs, const value& rhs) noexcept { return !(lhs < rhs); }

    inline void swap(value& lhs, value& rhs) noexcept { lhs.swap(rhs); }

} // namespace registry