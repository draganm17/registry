/** @file */
#pragma once

#include <cstdint>
#include <locale>
#include <string>
#include <typeinfo>
//#include <type_traits> // TODO: ???
#include <vector>

#include <registry/details/encoding.h>
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

        /*! TODO: ... */
        resource_list =               8,

        /*! TODO: ... */
        full_resource_descriptor =    9,

        /*! TODO: ... */
        resource_requirements_list =  10,
        
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

    //\cond HIDDEN_SYMBOLS
    namespace details
    {
        template <typename Sequence>
        std::vector<std::wstring> to_natives(const Sequence& src, const std::locale& loc)
        {
            throw 0;
            // TODO: ...
        }

        template <typename InputIt>
        std::vector<std::wstring> to_natives(InputIt first, InputIt last, const std::locale& loc)
        {
            throw 0;
            // TODO: ...
        }
    } //\endcond

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

        value& do_assign(value_type type, std::wstring&& val);

        value& do_assign(value_type type, std::vector<std::wstring>&& val);

    public:
        //! Default constructor.
        /*!
        //  @post `type() == value_type::none`.
        //
        //  @post `data() == nullptr`.
        //
        //  @post `size() == 0`.
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

        //! Constructs a value of type `value_type::sz`.
        /*!
        //  @tparam Source - TODO: ...
        //
        //  @post `type() == value_type::sz`.
        //
        //  @param[in] tag - value type tag.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @param[in] loc - TODO: ...
        */
        template <typename Source>
        value(sz_value_tag, const Source& val, const std::locale& loc = std::locale(""));

        //! Constructs a value of type `value_type::sz`.
        /*!
        //  @tparam InputIt - TODO: ...
        //
        //  @post `type() == value_type::sz`.
        //
        //  @param[in] first, last - TODO: ...
        //
        //  @param[in] loc         - TODO: ...
        */
        template <typename InputIt>
        value(sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        //! Constructs a value of type `value_type::expand_sz`.
        /*!
        //  @tparam Source - TODO: ...
        //
        //  @post `type() == value_type::expand_sz`.
        //
        //  @param[in] first, last - a string to be stored in this value.
        //
        //  @param[in] loc         - TODO: ...
        */
        // TODO: document the template
        template <typename Source>
        value(expand_sz_value_tag, const Source& val, const std::locale& loc = std::locale(""));

        //! Constructs a value of type `value_type::expand_sz`.
        /*!
        //  @tparam InputIt - TODO: ...
        //
        //  @post `type() == value_type::expand_sz`.
        //
        //  @param[in] first, last - TODO: ...
        //
        //  @param[in] loc         - TODO: ...
        */
        template <typename InputIt>
        value(expand_sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        //! Constructs a value of type `value_type::dword`.
        /*!
        //  @post `type() == value_type::dword`.
        //
        //  @post `to_uint32() == value`.
        //
        //  @param[in] val - an unsigned 32-bit integer to be stored in this value.
        */
        value(dword_value_tag, uint32_t val);

        //! Constructs a value of type `value_type::dword_big_endian`.
        /*!
        //  @post `type() == value_type::dword_big_endian`.
        //
        //  @post `to_uint32() == value`.
        //
        //  @param[in] val - an unsigned 32-bit integer to be stored in this value.
        */
        value(dword_big_endian_value_tag, uint32_t val);

        //! Constructs a value of type `value_type::link`.
        /*!
        //  @tparam Source - TODO: ...
        //
        //  @post `type() == value_type::link`.
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @param[in] loc - TODO: ...
        */
        template <typename Source>
        value(link_value_tag, const Source& val, const std::locale& loc = std::locale(""));

        //! Constructs a value of type `value_type::link`.
        /*!
        //  @tparam InputIt - TODO: ...
        //
        //  @post `type() == value_type::link`.
        //
        //  @param[in] first, last - TODO: ...
        //
        //  @param[in] loc         - TODO: ...
        */
        template <typename InputIt>
        value(link_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        //! Constructs a value of type `value_type::multi_sz`.
        /*!
        //  @tparam Sequence - TODO: ...
        //
        //  @post `type() == value_type::multi_sz`.
        //
        //  @param[in] val - a container, such as `Sequence::value_type` should be explicitly
        //                     convertible to `registry::string_view_type`.
        //
        //  @param[in] loc - TODO: ...
        */
        // TODO: rewrite description
        template <typename Sequence>
        value(multi_sz_value_tag, const Sequence& val, const std::locale& loc = std::locale(""));

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
        value(multi_sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        //! Constructs a value of type `value_type::qword`.
        /*!
        //  @post `type() == value_type::qword`.
        //
        //  @post `to_uint64() == value`.
        //
        //  @param[in] val - an unsigned 64-bit integer to be stored in this value.
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
        //  @post `this->type() == type`.
        //
        //  @post TODO: ...
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

    public:
        //! Returns the value type.
        value_type type() const noexcept;

        //! TODO: ...
        const void* data() const noexcept;

        //! TODO: ...
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
        std::string to_string(const std::locale& loc = std::locale("")) const;

        //! TODO: ...
        std::wstring to_wstring() const;

        //! Converts the value to an array of strings.
        /*!
        //  @throw `registry::bad_value_cast` if the value type is not `value_type::multi_sz`.
        */
        std::vector<std::string> to_strings(const std::locale& loc = std::locale("")) const;

        //! TODO: ...
        std::vector<std::wstring> to_wstrings() const;

    public:
        //! Replaces the contents of the value as if by `value(tag, val, loc).swap(*this)`.
        /*!
        //  @tparam Source - TODO: ...
        //
        //  @param[in] val   - a string to be stored in this value.
        //
        //  @param[in] loc   - TODO: ...
        //
        //  @return `*this`.
        */
        template <typename Source>
        value& assign(sz_value_tag tag, const Source& val, const std::locale& loc = std::locale(""));

        //! Replaces the contents of the value as if by `value(tag, first, last, loc).swap(*this)`.
        /*!
        //  @tparam InputIt - TODO: ...
        //
        //  @param[in] first, last - a string to be stored in this value.
        //
        //  @param[in] loc         - TODO: ...
        //
        //  @return `*this`.
        */
        template <typename InputIt>
        value& assign(sz_value_tag tag, InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        //! Replaces the contents of the value as if by `value(tag, val, loc).swap(*this)`.
        /*!
        //  @tparam Source - TODO: ...
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @param[in] loc - TODO: ...
        //
        //  @return `*this`.
        */
        template <typename Source>
        value& assign(expand_sz_value_tag tag, const Source& val, const std::locale& loc = std::locale(""));

        //! Replaces the contents of the value as if by `value(tag, first, last, loc).swap(*this)`.
        /*!
        //  @tparam InputIt - TODO: ...
        //
        //  @param[in] first, last - a string to be stored in this value.
        //
        //  @param[in] loc         - TODO: ...
        //
        //  @return `*this`.
        */
        template <typename InputIt>
        value& assign(expand_sz_value_tag tag, InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        //! Replaces the contents of the value as if by `value(tag, val).swap(*this)`.
        /*!
        //  @param[in] val - an unsigned 32-bit integer to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(dword_value_tag tag, uint32_t val);

        //! Replaces the contents of the value as if by `value(tag, val).swap(*this)`.
        /*!
        //  @param[in] val - an unsigned 32-bit integer to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(dword_big_endian_value_tag tag, uint32_t val);

        //! Replaces the contents of the value as if by `value(tag, val, loc).swap(*this)`.
        /*!
        //  @tparam Source - TODO: ...
        //
        //  @param[in] val - a string to be stored in this value.
        //
        //  @param[in] loc - TODO: ...
        //
        //  @return `*this`.
        */
        template <typename Source>
        value& assign(link_value_tag tag, const Source& val, const std::locale& loc = std::locale(""));

        //! Replaces the contents of the value as if by `value(tag, first, last, loc).swap(*this)`.
        /*!
        //  @tparam InputIt - TODO: ...
        //
        //  @param[in] first, last - a string to be stored in this value.
        //
        //  @param[in] loc         - TODO: ...
        //
        //  @return `*this`.
        */
        template <typename InputIt>
        value& assign(link_value_tag tag, InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        //! Replaces the contents of the value as if by `value(tag, val, loc).swap(*this)`.
        /*!
        //  @tparam Sequence - TODO: ...
        //
        //  @param[in] val - a container, such as `Sequence::value_type` should be explicitly
        //                   convertible to `registry::string_view_type`.
        //
        //  @param[in] loc - TODO: ...
        //
        //  @return `*this`.
        */
        // TODO: rewrite description
        template <typename Sequence>
        value& assign(multi_sz_value_tag, const Sequence& val, const std::locale& loc = std::locale(""));

        //! Replaces the contents of the value as if by `value(tag, first, last, loc).swap(*this)`.
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
        value& assign(multi_sz_value_tag, InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        //! Replaces the contents of the value as if by `value(tag, val).swap(*this)`.
        /*!
        //  @param[in] val - an unsigned 64-bit integer to be stored in this value.
        //
        //  @return `*this`.
        */
        value& assign(qword_value_tag, uint64_t val);

        //! Replaces the contents of the value as if by `value(type, data, size).swap(*this)`.
        /*!
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

    template <typename Source>
    inline value& value::assign(sz_value_tag, const Source& val, const std::locale& loc)
    {
        return do_assign(value_type::sz, to_native(val, loc));
    }

    template <typename InputIt>
    inline value& value::assign(sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        return do_assign(value_type::sz, to_native(first, last, loc));
    }

    template <typename Source>
    inline value& value::assign(expand_sz_value_tag, const Source& val, const std::locale& loc)
    {
        return do_assign(value_type::expand_sz, to_native(val, loc));
    }

    template <typename InputIt>
    inline value& value::assign(expand_sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        return do_assign(value_type::expand_sz, to_native(first, last, loc));
    }

    template <typename Source>
    inline value& value::assign(link_value_tag, const Source& val, const std::locale& loc)
    {
        return do_assign(value_type::link, to_native(val, loc));
    }

    template <typename InputIt>
    inline value& value::assign(link_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        return do_assign(value_type::link, to_native(first, last, loc));
    }

    template <typename Sequence>
    inline value& value::assign(multi_sz_value_tag, const Sequence& val, const std::locale& loc)
    {
        return do_assign(value_type::multi_sz, to_natives(val, loc));
    }

    template <typename InputIt>
    inline value& value::assign(multi_sz_value_tag, InputIt first, InputIt last, const std::locale& loc)
    {
        return do_assign(value_type::multi_sz, to_natives(first, last, loc));
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