/*!
## Registry library version 0.92 ##
The Registry library provides facilities for performing operations on Windows registry and its components, such as keys
and values. The registry library was inspired by the <a href="http://en.cppreference.com/w/cpp/filesystem">Filesystem</a>
library and tries to mimic its design whenever possible.
*/

/* TODO:
    - symlinks support.
    - import / export of registry keys ?
*/

#pragma once
#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <string>
#if _HAS_CXX17
#include <string_view>
#endif
#include <system_error>
#include <typeinfo>
#include <vector>

#if !_HAS_CXX17
#include <boost/utility/string_view.hpp> //awaiting for C++17 std::string_view
#endif


namespace registry
{
    enum class view          : uint32_t;
    enum class key_id        : uintptr_t;
    enum class value_type    : uint32_t;
    enum class access_rights : uint32_t;

    class bad_value_cast;
    class bad_weak_key_handle;
    class registry_error;
    class key;
    class value;
    class key_handle;
    class weak_key_handle;
    class key_entry;
    class value_entry;
    class key_iterator;
    class recursive_key_iterator;
    class value_iterator;

#if defined(_UNICODE)
    using string_type =      std::wstring;
#if _HAS_CXX17
    using string_view_type = std::wstring_view;
#else
    using string_view_type = boost::wstring_view;
#endif
#else
    using string_type =      std::string;
#if _HAS_CXX17
    using string_view_type = std::string_view;
#else
    using string_view_type = boost::string_view;
#endif
#endif
    using byte_array_type =      std::vector<uint8_t>;
#if _HAS_CXX17
    using byte_array_view_type = std::basic_string_view<uint8_t>;
    //TODO: replace with std::array_view<uint8_t> when (if ever) available
#else
    using byte_array_view_type = boost::basic_string_view<uint8_t>;
    //TODO: replace with std::array_view<uint8_t> when (if ever) available
#endif
    using key_time_type = std::chrono::time_point<std::chrono::system_clock>;

    /*! The system defines a set of predefined registry keys. Predefined keys help an application navigate in the
    registry and make it possible to develop tools that allow a system administrator to manipulate categories of data.
    For more information see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724836 */
    enum class key_id : uintptr_t 
    {
        // TODO: ...
        none =                         0x00000000,

        /*! Registry entries subordinate to this key define types (or classes) of documents and the properties 
        associated with those types. */
        classes_root =                 0x80000000,

        /*! Registry entries subordinate to this key define the preferences of the current user. */
        current_user =                 0x80000001,

        /*! Registry entries subordinate to this key define the physical state of the computer, including data about 
        the bus type, system memory, and installed hardware and software. */
        local_machine =                0x80000002,

        /*! Registry entries subordinate to this key define the default user configuration for new users on the local 
        computer and the user configuration for the current user. */
        users =                        0x80000003,

        /*! Registry entries subordinate to this key allow you to access performance data. */
        performance_data =             0x80000004,

        /*! Registry entries subordinate to this key reference the text strings that describe counters in 
        US English. */
        performance_text =             0x80000050,

        /*! Registry entries subordinate to this key reference the text strings that describe counters in the local 
        language of the area in which the computer system is running. */
        performance_nlstext =          0x80000060,

        /*! Contains information about the current hardware profile of the local computer system. */
        current_config =               0x80000005,

        /*! Registry entries subordinate to this key define preferences of the current user that are local to the 
        machine. */
        current_user_local_settings =  0x80000007
    };

    /*! On 64-bit Windows, portions of the registry entries are stored separately for 32-bit application and 64-bit
    applications and mapped into separate logical registry views using the registry redirector and registry reflection,
    because the 64-bit version of an application may use different registry keys and values than the 32-bit version. 
    These flags enable explicit access to the 64-bit registry view and the 32-bit view, respectively. For more 
    information see: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724072 */
    enum class view : uint32_t
    {
        /*! Access a 64-bit key from either a 32-bit or 64-bit application */
        view_32bit =    0x0200,

        /*! Access a 32-bit key from either a 32-bit or 64-bit application.
        Ignored on the 32-bit versions of Windows. */
        view_64bit =    0x0100
    };

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

    /*! The Windows security model enables you to control access to registry keys. For more information see:
    https://msdn.microsoft.com/en-us/library/windows/desktop/ms724878 \n
    access_rights satisfies the requirements of BitmaskType (which means the bitwise operators `operator&`, 
    operator|`, `operator^`, `operator~`, `operator&=`, `operator|=`, and `operator^=` are defined for this type) */
    enum class access_rights : uint32_t
    {
        /*! TODO: ... */
        all_access =           0x000F003F,

        /*! TODO: ... */
        create_link =          0x00000020,

        /*! TODO: ... */
        create_sub_key =       0x00000004,

        /*! TODO: ... */
        enumerate_sub_keys =   0x00000008,

        /*! TODO: ... */
        execute =              0x00020019,

        /*! TODO: ... */
        notify =               0x00000010,

        /*! TODO: ... */
        query_value =          0x00000001,

        /*! TODO: ... */
        read =                 0x00020019,

        /*! TODO: ... */
        set_value =            0x00000002,

        /*! TODO: ... */
        write =                0x00020006,

        /*! TODO: ... */
        unknown =              0x0
    };

    /*! This type represents available options that control the behavior of the recursive_key_iterator. \n
    key_options satisfies the requirements of BitmaskType (which means the bitwise operators `operator&`, 
    `operator|`, `operator^`, `operator~`, `operator&=`, `operator|=`, and `operator^=` are defined for this type) */
    // TODO: integrate this to the recursive_key_iterator
    enum class key_options : uint16_t
    {
        /*! (Default) Permission denied is error. */
        none =                    0x0,

        /*! Skip keys that would otherwise result in permission denied errors. */
        skip_permission_denied =  0x1
    };

    /*! The Windows security model enables you to control access to registry keys. For more information see:
    https://msdn.microsoft.com/en-us/library/windows/desktop/ms724878 \n
    key_info_mask satisfies the requirements of BitmaskType (which means the bitwise operators `operator&`, 
    `operator|`, `operator^`, `operator~`, `operator&=`, `operator|=`, and `operator^=` are defined for this type) */
    enum class key_info_mask : uint16_t
    {
        /*! TODO: ... */
        none =                      0x0000,

        /*! TODO: ... */
        read_subkeys =              0x0001,

        /*! TODO: ... */
        read_values =               0x0002,

        /*! TODO: ... */
        read_max_subkey_size =      0x0004,

        /*! TODO: ... */
        read_max_value_name_size =  0x0008,

        /*! TODO: ... */
        read_max_value_data_size =  0x0010,

        /*! TODO: ... */
        read_last_write_time =      0x0020,

        /*! TODO: ... */
        all =                       0x003F
    };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct none_value_tag             { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct sz_value_tag               { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct expand_sz_value_tag        { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct binary_value_tag           { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct dword_value_tag            { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct dword_big_endian_value_tag { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct link_value_tag             { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct multi_sz_value_tag         { };

    //! Defines a type of object to be used to select an overload of registry::value constructor or assign function.
    struct qword_value_tag            { };

    //! Defines a type of object that stores information about a registry key.
    struct key_info
    {
        /*! The number of subkeys that are contained by the key. */
        uint32_t       subkeys;

        /*! The number of values that are associated with the key. */
        uint32_t       values;

        /*!  The size of the key's subkey with the longest name, in characters, not including the terminating 
        null character. */
        uint32_t       max_subkey_size;

        /*! The size of the key's longest value name, in characters, not including the terminating null character. */
        uint32_t       max_value_name_size;

        /*! The size of the longest data component among the key's values, in bytes. */
        uint32_t       max_value_data_size;

        /*! The last time that the key or any of its value entries is modified. */
        key_time_type  last_write_time;
    };

    //------------------------------------------------------------------------------------//
    //                             class bad_value_cast                                   //
    //------------------------------------------------------------------------------------//

    //! Defines a type of object to be thrown by registry::value conversion functions on failure.
    class bad_value_cast
        : public std::bad_cast
    {
    public:
        const char* what() const noexcept override { return "registry::bad_value_cast"; }
    };

    //------------------------------------------------------------------------------------//
    //                           class bad_weak_key_handle                                //
    //------------------------------------------------------------------------------------//

    /*! \brief
    Defines a type of the object thrown by the constructors of registry::key_handle that take
    registry::weak_key_handle as the argument, when the registry::weak_key_handle is already expired. */
    class bad_weak_key_handle
        : public std::exception
    {
    public:
        const char* what() const noexcept override { return "registry::bad_weak_key_handle"; }
    };

    //------------------------------------------------------------------------------------//
    //                             class registry_error                                   //
    //------------------------------------------------------------------------------------//

    /*! \brief 
    Defines an exception object that is thrown on failure by the overloads of registry library functions
    not having an argument of type std::error_code&. */
    class registry_error 
        : public std::system_error
    {
        struct storage;
        std::shared_ptr<storage> m_info;

    public:
        //! Constructs a new registry error object. The explanatory string is set to `msg`, error code is set to `ec`.
        registry_error(std::error_code ec, const std::string& msg);

        /*! \brief
        Constructs a new registry error object. The explanatory string is set to `msg`, error code is set to `ec`,
        the first key is set to `key1`.*/
        registry_error(std::error_code ec, const std::string& msg, 
                       const key& key1);

        /*! \brief
        Constructs a new registry error object. The explanatory string is set to `msg`, error code is set to `ec`,
        the first key is set to `key1`, the second key is set to `key2`.*/
        registry_error(std::error_code ec, const std::string& msg, 
                       const key& key1, const key& key2);
        
        /*! \brief
        Constructs a new registry error object. The explanatory string is set to `msg`, error code is set to `ec`,
        the first key is set to `key1`, the second key is set to `key2`, the value name is set to `value_name`.*/
        registry_error(std::error_code ec, const std::string& msg,
            const key& key1, const key& key2, string_view_type value_name);

    public:
        //! Returns the first key that was stored in the exception object.
        const key& key1() const noexcept;

        //! Returns the second key that was stored in the exception object.
        const key& key2() const noexcept;

        //! Returns the value name that was stored in the exception object.
        const string_type& value_name() const noexcept;
    };

    //\cond HIDDEN_SYMBOLS
    namespace details
    {
        struct key_state
        {
            view         m_view;
            string_type  m_name;
        };

        struct key_iterator_state
        {
            string_view_type  m_value;
            string_view_type  m_key_string_view;
        };
    } //\endcond

    //------------------------------------------------------------------------------------//
    //                                   class key                                        //
    //------------------------------------------------------------------------------------//

    //! Represents a registry key.
    /*!
    Objects of type registry::key represent keys on the Windows registry. Only syntactic aspects of keys are handled: 
    the key name may represent a non-existing key or even one that is not allowed to exist on Windows.\n
    A key is composed of two parts: registry::view and the key::name(). The latter has the following syntax:
    1. root key (optional): the string representation of one of the predefined keys identifies (registry::key_id).
    2. subkey (optional): ...
    
    */
    class key 
        : private details::key_state
    {
    public:
        class iterator;
        using const_iterator = iterator;

    public:
        /*! \brief
        The value of type registry::view which is passed to registry::key constructor by default. Is equal to
        registry::view::view_32bit for 32-bit applications and registry::view::view_64bit for 64-bit applications. */
        static const view default_view;

    public:
        static key from_key_id(key_id id);

    public:
        //! Default constructor.
        /*!
        Equivalent to `key(key_id::none)`.
        @post `*this == other`.
        */
        key() noexcept;

        //! Constructs the key with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        key(const key& other) = default;

        /*! \brief
        Constructs the key with the contents of `other` using move semantics. `other` is left in a valid but
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        key(key&& other) noexcept = default;

        //! Constructs the key from a key name string and a registry view.
        /*!
        @param[in] name - a key name string.
        @param[in] view - a registry view.
        */
        key(string_view_type name, view view = default_view);

        // TODO: ...
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<string_view_type, Source>::value>
        >
        key(const Source& name, view view = default_view) : key(string_view_type(name), view) { }

        template <typename InputIt,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, 
                                                                    std::iterator_traits<InputIt>::value_type>::value>
        >
        key(InputIt first, InputIt last, registry::view view = default_view) { /* TODO: ... */ }

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        key& operator=(const key& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key& operator=(key&& other) noexcept = default;

    public:
        //! Returns the name of the key containing the predefined key identifier string followed by the subkey string.
        const string_type& name() const noexcept;

        //! Returns the registry view of the key.
        view view() const noexcept;

        //! Returns the root component of the key.
        /*!
        Equivalent to `has_root_key() ? key(*begin(), view()) : key(string_view_type(), view())`.
        */
        key root_key() const;

        //! Returns the leaf component of the key.
        /*!
        Equivalent to `has_leaf_key() ? key(*--end(), view()) : key(string_view_type(), view())`.
        */
        key leaf_key() const;

        //! Returns the parent of the key.
        /*!
        Equivalent to `has_parent_key() ? key(begin(), --end(), view()) : key(string_view_type(), view())`.
        */
        key parent_key() const;

        //! Checks if the key has a root key.
        /*!
        Equivalent to `begin() != end()`.
        */
        bool has_root_key() const noexcept;

        //! Checks if the key has a leaf key.
        /*!
        Equivalent to `begin() != end()`.
        */
        bool has_leaf_key() const noexcept;

        //! Checks if the key has a parent key.
        /*!
        Equivalent to `!name().empty() && ++begin() != end()`.
        */
        bool has_parent_key() const noexcept;

        //! Checks whether the key is absolute.
        /*!
        An absolute key is a key that unambiguously identifies the location of a registry key. The name of such key
        should begin with a predefined key identifier. \n
        Examples:
        - "HKEY_LOCAL_MACHINE\Software\Microsoft" is an absolute key because it begins with "HKEY_LOCAL_MACHINE";
        - "Software\Microsoft" is an relative key, because it does not begin with a predefined key identifier.
        */
        bool is_absolute() const noexcept;

        //! Checks whether the key is relative.
        /*!
        Equivalent to `!is_absolute()`.
        */
        bool is_relative() const noexcept;

        //! Compares key objects.
        /*!
        - if `view() < other.view()`, `*this` is less than `other`;
        - otherwise if `view() > other.view()`, `*this` is greater than `other`;
        - otherwise keys name components are compared lexicographically. The comparison is case-insensitive.

        @return
            A value less than 0 if the key is less than the given key.\n
            A value equal to 0 if the key is equal to the given key.\n
            A value greater than 0 if the key is greater than the given key.
        */
        int compare(const key& other) const noexcept;

        /*! \brief
        Returns an iterator to the first component of the key name. If the key name is empty, the returned iterator
        is equal to end(). */
        iterator begin() const noexcept;

        /*! \brief
        Returns an iterator one past the last component of the key name. Dereferencing this iterator is undefined 
        behavior. */
        iterator end() const noexcept;

    public:
        //! Replaces the contents of the key.
        /*!
        @post `*this == key(name, view)`.
        @param[in] name - a key name string.
        @param[in] view - a registry view.
        @return `*this`.
        */
        key& assign(string_view_type name, registry::view view = default_view);

        // TODO: ...
        template <typename InputIt,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, 
                                                                    std::iterator_traits<InputIt>::value_type>::value>
        >
        key& assign(InputIt first, InputIt last, registry::view view = default_view) { return *this; /* TODO: ... */ }

        //! Appends elements to the key name.
        /*!
        First, appends the key separator to the key name, except if any of the following conditions is true:
        - the separator would be redundant (the key name already ends with a separator);
        - the key name is empty;
        - `subkey` is an empty string;
        - `subkey` begins with a key separator.

        Then, appends `subkey` to the key name.
        @return `*this`.
        */
        key& append(string_view_type subkey);

        //! Concatenates the key name with `subkey` without introducing a key separator.
        /*!
        Equivalent to `*this = key(name().append(subkey.data(), subkey.size()), view())`.
        @return `*this`.
        */
        key& concat(string_view_type subkey);

        //! Removes a single leaf component.
        /*!
        Equivalent to `*this = parent_key()`.
        @pre `has_leaf_key() == true`.
        @return `*this`.
        */
        key& remove_leaf();

        //! Replaces a single leaf component with `replacement`.
        /*!
        Equivalent to `remove_leaf().append(replacement)`.
        @pre `has_leaf_key() == true`.
        @return `*this`.
        */
        key& replace_leaf(string_view_type replacement);

        //! Swaps the contents of `*this` and `other`.
        void swap(key& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                             class key::iterator                                    //
    //------------------------------------------------------------------------------------//

    //! A constant BidirectionalIterator with a value_type of registry::string_view_type.
    class key::iterator 
        : private details::key_iterator_state
    {
        string_view_type  m_value;
        string_view_type  m_key_string_view;

    public:
        using value_type =        string_view_type;
        using difference_type =   std::ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        bool operator==(const iterator& rhs) const noexcept;

        bool operator!=(const iterator& rhs) const noexcept;

        reference operator*() const;

        pointer operator->() const;

    public:
        iterator& operator++();

        iterator operator++(int);

        iterator& operator--();

        iterator operator--(int);
    };

    //\cond HIDDEN_SYMBOLS
    namespace details
    {
        struct value_state
        {
            value_type       m_type;
            byte_array_type  m_data;
        };

        //template <typename It>
        //auto make_string_enumerator(It first, It last) -> decltype(auto)
        //{
        //    return [first = std::move(first), last = std::move(last)](string_view_type &val) mutable -> bool {
        //        return first != last ? (val = string_view_type(*first++), true) : false;
        //    };
        //}
    } //\endcond

    //------------------------------------------------------------------------------------//
    //                                 class value                                        //
    //------------------------------------------------------------------------------------//

    //! Represents a registry value.
    class value 
        : private details::value_state
    {
    private:
        value& assign_impl(multi_sz_value_tag, const std::function<bool(string_view_type&)>& enumerator);

    public:
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

        //! Constructs a value of type value_type::none.
        /*!
        @post `type() == value_type::none`.
        @post `data().empty() == true`.
        @param[in] tag - value type tag.
        */
        explicit value(none_value_tag tag = {}) noexcept;

        //! Constructs a value of type value_type::sz.
        /*!
        @post `type() == value_type::sz`.
        @post `to_string() == value`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        */
        value(sz_value_tag tag, string_view_type value);

        //! Constructs a value of type value_type::expand_sz.
        /*!
        @post `type() == value_type::expand_sz`.
        @post `to_string() == value`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        */
        value(expand_sz_value_tag tag, string_view_type value);

        //! Constructs a value of type value_type::binary.
        /*!
        @post `type() == value_type::binary`.
        @post `to_byte_array() == value`.
        @param[in] tag - value type tag.
        @param[in] value - binary data to be stored in this value.
        */
        value(binary_value_tag tag, byte_array_view_type value);

        //! Constructs a value of type value_type::dword.
        /*!
        @post `type() == value_type::dword`.
        @post `to_uint32() == value && to_uint64() == value`.
        @param[in] tag - value type tag.
        @param[in] value - an unsigned 32-bit integer to be stored in this value.
        */
        value(dword_value_tag tag, uint32_t value);

        //! Constructs a value of type value_type::dword_big_endian.
        /*!
        @post `type() == value_type::dword_big_endian`.
        @post `to_uint32() == value && to_uint64() == value`.
        @param[in] tag - value type tag.
        @param[in] value - an unsigned 32-bit integer to be stored in this value.
        */
        value(dword_big_endian_value_tag tag, uint32_t value);

        //! Constructs a value of type value_type::link.
        /*!
        @post `type() == value_type::link`.
        @post `to_string() == value`.
        @param[in] tag - value type tag.
        @param[in] value - a string to be stored in this value.
        */
        value(link_value_tag tag, string_view_type value);

        //! Constructs a value of type value_type::multi_sz.
        /*!
        @post `type() == value_type::multi_sz`.
        @post TODO: ...
        @param[in] tag - value type tag.
        @param[in] value - a container, such as Sequence::value_type should be explicitly convertible to 
                           registry::string_view_type.
        */
        template <typename Sequence,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, Sequence::value_type>::value>
        >
        value(multi_sz_value_tag tag, const Sequence& value);

        //! Constructs a value of type value_type::multi_sz.
        /*!
        @post `type() == value_type::multi_sz`.
        @post TODO: ...
        @param[in] tag - value type tag.
        @param[in] first, last - input iterators, such as std::iterator_traits<InputIt>::value_type should be 
                                 explicitly convertible to registry::string_view_type.
        */
        template <typename InputIt,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, 
                                                                    std::iterator_traits<InputIt>::value_type>::value>
        >
        value(multi_sz_value_tag tag, InputIt first, InputIt last);

        //! Constructs a value of type value_type::multi_sz.
        /*!
        @post `type() == value_type::multi_sz`.
        @post TODO: ...
        @param[in] tag - value type tag.
        @param[in] init - an object of type std::initializer_list<T>, such as T should be explicitly convertible to 
                          registry::string_view_type.
        */
        template <typename T,
                  typename = std::enable_if_t<std::is_constructible<string_view_type, T>::value>
        >
        value(multi_sz_value_tag tag, std::initializer_list<T> init);

        //! Constructs a value of type value_type::qword.
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
        byte sequence is not sutable for representing the value of a given type, then calling a conversion function 
        applicable to that type may produce a valid but undefined result. If the value type is one of value_type::sz, 
        value_type::expand_sz, value_type::link or value_type::multi_sz, providing the null terminator character is 
        desirable but not necessary.
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
        @throw registry::bad_value_cast if the value type is not one of value_type::dword or 
               value_type::dword_big_endian.
        */
        uint32_t to_uint32() const;

        //! Converts the value to an unsigned 64-bit integer.
        /*!
        @throw registry::bad_value_cast if the value type is not one of value_type::dword, 
               value_type::dword_big_endian or value_type::qword.
        */
        uint64_t to_uint64() const;

        //! Converts the value to a string.
        /*!
        @throw registry::bad_value_cast if the value type is not one of value_type::sz, value_type::expand_sz or 
               value_type::link.
        */
        string_type to_string() const;

        //! Converts the value to an array of strings.
        /*!
        @throw registry::bad_value_cast if the value type is not value_type::multi_sz.
        */
        std::vector<string_type> to_strings() const;

        //! Converts the value to a binary data array.
        /*!
        @throw registry::bad_value_cast if the value type is not value_type::binary.
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
        @param[in] value - a container, such as Sequence::value_type should be explicitly convertible to 
                           registry::string_view_type.
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
        @param[in] first, last - input iterators, such as std::iterator_traits<InputIt>::value_type should be 
                                 explicitly convertible to registry::string_view_type.
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
        @param[in] init - an object of type std::initializer_list<T>, such as T should be explicitly convertible to 
                          registry::string_view_type.
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

    public:
        //! Swaps the contents of `*this` and `other`.
        void swap(value& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                                class key_handle                                    //
    //------------------------------------------------------------------------------------//

    //! Represents a handle to an registry key.
    /*!
    registry::key_handle is a wrapper around a native key handle that retains shared ownership of that handle. Several 
    key_handle objects may own the same key handle. The object is destroyed and its handle is closed when either of the
    following happens:
    - the last remaining key_handle owning the key handle is destroyed;
    - the last remaining key_handle owning the key handle is assigned another handle via operator=.

    A key_handle may also own no handle, in which case it is called `invalid`.
    */
    // TODO: describe the internal umplementation ???
    class key_handle
    {
        friend class weak_key_handle;

        struct state;
        std::shared_ptr<state> m_state;

    public:
        using native_handle_type = uintptr_t;

    public:
        //! Default constructor.
        /*!
        @post `valid() == false`.
        */
        constexpr key_handle() noexcept = default;

        /*! \brief
        Constructs a key_handle which shares ownership of the handle managed by `other`. If `other` manages no
        handle, `*this` manages no handle too. */
        key_handle(const key_handle& other) noexcept = default;

        //! Constructs the handle with the contents of `other` using move semantics.
        /*!
        @post `other.valid() == false`.
        @post `*this` has the original value of `other`.
        */
        key_handle(key_handle&& other) noexcept = default;

        // TODO: ...
        key_handle(const weak_key_handle& handle);

        // TODO: ...
        key_handle(key_id id, access_rights rights = access_rights::unknown);

        // TODO: ...
        key_handle(native_handle_type handle, const registry::key& key, access_rights rights);

        /*! \brief
        Replaces the managed handle with the one managed by `other`. If `*this` already owns an handle and it 
        is the last key_handle owning it, and `other` is not the same as `*this`, the owned handle is closed. */
        /*!
        @return `*this`.
        */
        key_handle& operator=(const key_handle& other) noexcept = default;

        //! Replaces the contents of `*this` with those of `other` using move semantics.
        /*!
        @post `other.valid() == false`.
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key_handle& operator=(key_handle&& other) noexcept = default;

    public:
        //! Returns the key this handle was constructed with.
        /*!
        @throw TODO: ...
        */
        registry::key key() const;

        //! Returns the access rights this handle was constructed with.
        /*!
        @throw TODO: ...
        */
        access_rights rights() const noexcept;

        //! Returns the underlying implementation-defined native handle object suitable for use with WinAPI.
        /*!
        @throw TODO: ...
        */
        native_handle_type native_handle() const noexcept;

        // TODO: ...
        bool valid() const noexcept;

    public:
        //! Check whether the registry key specified by this handle contains the given value.
        /*!
        The key must have been opened with the access_rights::query_value access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the
                                default value.
        @return `true` if the given name corresponds to an existing registry value, `false` otherwise.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
               `this->key()` and the value name set to `value_name`. std::bad_alloc may be thrown if memory 
               allocation fails.
        */
        bool exists(string_view_type value_name);

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `false` on error.
        */
        bool exists(string_view_type value_name, std::error_code& ec);

        //! Retrieves information about the registry key specified by this handle.
        /*!
        The key must have been opened with the access_rights::query_value access right.
        @param[in] mask - a mask specifying which fields of key_id structure are filled out and which aren't.
                          The fields of key_id whick aren't filled out will have default-constructed values.
        @return an instance of key_info.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
               `this->key()`. std::bad_alloc may be thrown if memory allocation fails.
        */
        key_info info(key_info_mask mask) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `key_info()` on error.
        */
        key_info info(key_info_mask mask, std::error_code& ec) const;

        //! Reads the content of an registry value contained inside the registry key specified by this handle.
        /*!
        The key must have been opened with the access_rights::query_value access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the 
                                default value.
        @return An instance of registry::value.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
               `this->key()` and the value name set to `value_name`. std::bad_alloc may be thrown if memory 
               allocation fails.
        */
        value read_value(string_view_type value_name) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument. 
        /*!
        Returns an default-constructed value on error.
        */
        value read_value(string_view_type value_name, std::error_code& ec) const;

        //! Creates a subkey inside the registry key specified by this handle.
        /*!
        If the key already exists, the function opens it. The function creates all missing keys in the specified path. \n
        The calling process must have access_rights::create_sub_key access to the key specified by this handle. The 
        access rights the key was opened with does not affect the operation.
        @param[in] subkey - an relative key specifying the subkey that this function opens or creates. If the subkey
                            name is an empty string the function will return a new handle to the key specified by this
                            handle.
        @param[in] rights - the access rights for the key to be created.
        @return a pair consisting of an handle to the opened or created key and a `bool` denoting whether the key 
                was created.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
               `this->key()` and the second key set to `subkey`. std::bad_alloc may be thrown if memory allocation 
               fails.
        */
        std::pair<key_handle, bool> create_key(const registry::key& subkey, access_rights rights) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `std::make_pair(key_handle(), false)` on error.
        */
        std::pair<key_handle, bool> create_key(const registry::key& subkey, access_rights rights, std::error_code& ec) const;

        //! Writes an value to the registry key specified by this handle.
        /*!
        The key must have been opened with the access_rights::set_value access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the
                                default value.
        @param[in] value - the content of the value.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to 
               `this->key()` and the value name set to `value_name`. std::bad_alloc may be thrown if memory 
               allocation fails.
        */
        void write_value(string_view_type value_name, const value& value) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        void write_value(string_view_type value_name, const value& value, std::error_code& ec) const;

        //! Deletes an subkey from the registry key specified by this handle.
        /*!
        The subkey to be deleted must not have subkeys. To delete a key and all its subkeys use `remove_all` function. \n
        The access rights of this key do not affect the delete operation.
        @param[in] subkey - an relative key specifying the subkey that this function deletes.
        @return `true` if the subkey was deleted, `false` if it did not exist.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
               `this->key()` and the second key set to `subkey`. std::bad_alloc may be thrown if memory allocation 
               fails.
        */
        bool remove(const registry::key& subkey) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `false` on error.
        */
        bool remove(const registry::key& subkey, std::error_code& ec) const;

        //! Deletes an registry value from the registry key specified by this handle.
        /*!
        @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the
                                default value.
        @return `true` if the value was deleted, `false` if it did not exist.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
               `this->key()` and the value name set to `value_name`. std::bad_alloc may be thrown if memory 
               allocation fails.
        */
        bool remove(string_view_type value_name) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `false` on error.
        */
        bool remove(string_view_type value_name, std::error_code& ec) const;

        // TODO: ...
        std::uintmax_t remove_all(const registry::key& subkey) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument. 
        /*!
        Returns `static_cast<std::uintmax_t>(-1)` on error.
        */
        std::uintmax_t remove_all(const registry::key& subkey, std::error_code& ec) const;

        /*! \brief 
        Checks whether the registry key specified by this handle and the registry key specified by 
        `key` refer to the same registry key. */
        /*!
        The key must have been opened with the access_rights::query_value access right.
        @param[in] key - an absolute registry key.
        @return `true` if `*this` and `key` resolve to the same registry key, else `false`.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to 
               `this->key()` and the second key set to `key`. std::bad_alloc may be thrown if memory allocation fails.
        */
        bool equivalent(const registry::key& key) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `false` on error.
        */
        bool equivalent(const registry::key& key, std::error_code& ec) const;

        /*! \brief 
        Checks whether the registry key specified by this handle and the registry key specified by 
        `handle` refer to the same registry key. */
        /*!
        Both keys must have been opened with the access_rights::query_value access right.
        @param[in] handle - a handle to an opened registry key.
        @return `true` if `*this` and `handle` resolve to the same registry key, else `false`.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to 
               `this->key()` and the second key set to `handle.key()`. std::bad_alloc may be thrown if memory 
               allocation fails.
        */
        bool equivalent(const key_handle& handle) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `false` on error.
        */
        bool equivalent(const key_handle& handle, std::error_code& ec) const;

    public:
        //! Swaps the contents of `*this` and `other`.
        void swap(key_handle& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                            class weak_key_handle                                   //
    //------------------------------------------------------------------------------------//

    //! Represents a weak reference to a registry key handle managed by registry::key_handle.
    /*!
    registry::weak_key_handle is a wrapper around a native key handle that holds a non-owning ("weak") reference to
    a key handle that is managed by registry::key_handle. It must be converted to registry::key_handle in order to 
    access the referenced handle.
    */
    // TODO: describe the internal umplementation ???
    class weak_key_handle
    {
        friend class key_handle;

        std::weak_ptr<key_handle::state> m_state;

    public:
        //! Default constructor. Constructs an invalid weak_key_handle.
        /*!
        @post `expired() == true`.
        */
        constexpr weak_key_handle() noexcept = default;

        /*! \brief
        Constructs a weak_key_handle which shares ownership of the handle managed by `other`. If `other` manages no 
        handle, `*this` manages no handle too. */
        weak_key_handle(const weak_key_handle& other) noexcept = default;

        //! Constructs the handle with the contents of `other` using move semantics.
        /*!
        @post `other.expired() == true`.
        @post `*this` has the original value of `other`.
        */
        weak_key_handle(weak_key_handle&& other) noexcept = default;

        // TODO: ...
        weak_key_handle(const key_handle& handle) noexcept;

        /*! \brief
        Replaces the managed handle with the one managed by `other`. The handle is shared with `other`. If `other` 
        manages no handle, `*this` manages no handle too. */
        /*!
        @return `*this`.
        */
        weak_key_handle& operator=(const weak_key_handle& other) noexcept = default;

        //! Replaces the contents of `*this` with those of `other` using move semantics.
        /*!
        @post `other.expired() == true`.
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        weak_key_handle& operator=(weak_key_handle&& other) noexcept = default;

        // TODO: ...
        weak_key_handle& operator=(const key_handle& other) noexcept;

    public:
        //! Checks whether the referenced handle was already closed.
        bool expired() const noexcept;

        //! Creates a key_handle that manages the referenced handle. 
        /*!
        If there is no managed handle, i.e. `*this` is invalid, then the returned key_handle also is invalid.
        */
        key_handle lock() const noexcept;

    public:
        //! Swaps the contents of `*this` and `other`.
        void swap(weak_key_handle& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                               class key_entry                                      //
    //------------------------------------------------------------------------------------//

    class key_entry 
        //: private details::key_entry_state
    {
        friend class key_iterator;
        friend class recursive_key_iterator;

        key              m_key;
        weak_key_handle  m_key_handle;

    public:
        // TODO: ...
        key_entry() noexcept = default;

        //! Constructs the value with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        key_entry(const key_entry& other) = default;

        /*! \brief
        Constructs the value with the contents of `other` using move semantics. `other` is left in a valid but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        key_entry(key_entry&& other) noexcept = default;

        // TODO: ...
        key_entry(const key& key);

        // TODO: ...
        key_entry(const key_handle& handle);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        key_entry& operator=(const key_entry& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, 
        but unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key_entry& operator=(key_entry&& other) noexcept = default;

    public:
        //! Returns the key that was stored in the value entry object.
        const key& key() const noexcept;

        // TODO: ...
        key_info info(key_info_mask mask) const;

        // TODO: ...
        key_info info(key_info_mask mask, std::error_code& ec) const;

    public:
        // TODO: ...
        key_entry& assign(const registry::key& key);

        // TODO: ...
        key_entry& assign(const registry::key_handle& handle);

        //! Swaps the contents of `*this` and `other`.
        void swap(key_entry& other) noexcept;
    };

    //\cond HIDDEN_SYMBOLS
    namespace details
    {
        //struct value_entry_state
        //{
        //    key          m_key;
        //    string_type  m_value_name;
        //};
    } //\endcond

    //------------------------------------------------------------------------------------//
    //                              class value_entry                                     //
    //------------------------------------------------------------------------------------//

    class value_entry 
        //: private details::value_entry_state
    {
        friend class value_iterator;

        key              m_key;
        string_type      m_value_name;
        weak_key_handle  m_key_handle;

    public:
        // TODO: ...
        value_entry() noexcept = default;

        //! Constructs the value with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        value_entry(const value_entry& other) = default;

        /*! \brief
        Constructs the value with the contents of `other` using move semantics. `other` is left in a valid but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        value_entry(value_entry&& other) noexcept = default;

        // TODO: ...
        value_entry(const key& key, string_view_type value_name);

        // TODO: ...
        value_entry(const key_handle& handle, string_view_type value_name);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        value_entry& operator=(const value_entry& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, 
        but unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        value_entry& operator=(value_entry&& other) noexcept = default;

    public:
        //! Returns the key that was stored in the value entry object.
        const key& key() const noexcept;

        //! Returns the value name that was stored in the value entry object.
        const string_type& value_name() const noexcept;

        // TODO: ...
        registry::value value() const;

        /*!
        Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        Returns a default-constructed value on error.
        */
        registry::value value(std::error_code& ec) const;

    public:
        // TODO: ...
        value_entry& assign(const registry::key& key, string_view_type value_name);

        // TODO: ...
        value_entry& assign(const registry::key_handle& handle, string_view_type value_name);

        //! Swaps the contents of `*this` and `other`.
        void swap(value_entry& other) noexcept;
    };
    
    //------------------------------------------------------------------------------------//
    //                              class key_iterator                                    //
    //------------------------------------------------------------------------------------//

    //! An iterator to the contents of the registry key.
    /*!
    key_iterator is an InputIterator that iterates over the key elements of a registry key (but does not visit the 
    subkeys). The iteration order is unspecified, except that each entry is visited only once. If the key_iterator is 
    advanced past the last entry, it becomes equal to the default-constructed iterator, also known as the end iterator.
    Two end iterators are always equal, dereferencing or incrementing the end iterator is undefined behavior. If an 
    entry is deleted or added to the key tree after the key iterator has been created, it is unspecified whether the 
    change would be observed through the iterator. 
    */
    class key_iterator
    {
        uint32_t                              m_idx;
        key_handle                            m_hkey;
        key_entry                             m_entry;
        std::vector<string_type::value_type>  m_buffer;

    public:
        using value_type =        key_entry;
        using difference_type =   ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::input_iterator_tag;

    public:
        //! Constructs the end iterator.
        key_iterator() noexcept = default;

        //! Constructs the iterator with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        key_iterator(const key_iterator& other) = default;

        /*! \brief
        Constructs the iterator with the contents of `other` using move semantics. `other` is left in a valid but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        key_iterator(key_iterator&& other) noexcept = default;

        //! Constructs a iterator that refers to the first subkey of a registry key specified by `key`. 
        /*!
        If `key` refers to an non-existing registry key, returns the end iterator. 
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
               std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit key_iterator(const key& key);

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Constructs an end iterator on error.
        */
        key_iterator(const key& key, std::error_code& ec);

        //! Constructs a iterator that refers to the first subkey of a registry key specified by `handle`. 
        /*!
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
        `handle.key()`. std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit key_iterator(const key_handle& handle);

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Constructs an end iterator on error.
        */
        key_iterator(const key_handle& handle, std::error_code& ec);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        key_iterator& operator=(const key_iterator& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, 
        but unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key_iterator& operator=(key_iterator&& other) noexcept = default;

    public:
        // TODO: ...
        bool operator==(const key_iterator& rhs) const noexcept;

        // TODO: ...
        bool operator!=(const key_iterator& rhs) const noexcept;

        //! Accesses the pointed-to registry::key.
        /*!
        @pre `*this != key_iterator()`.
        @return Value of the key referred to by this iterator.
        */
        reference operator*() const;

        //! Accesses the pointed-to registry::key.
        /*!
        @pre `*this != key_iterator()`.
        @return Pointer to the key referred to by this iterator.
        */
        pointer operator->() const;

    public:
        //! Calls increment(), then returns `*this`.
        /*!
        @pre `*this != key_iterator()`.
        @throw registry::registry_error on underlying OS API errors. std::bad_alloc may be thrown if memory allocation 
               fails.
        */
        key_iterator& operator++();

        //! Makes a copy of `*this`, calls increment(), then returns the copy.
        /*!
        @pre `*this != key_iterator()`.
        @throw registry::registry_error on underlying OS API errors. std::bad_alloc may be thrown if memory allocation 
               fails.
        */
        key_iterator operator++(int);

        //! Advances the iterator to the next entry.
        /*!
        If an error occurs `*this` is becoming equal to the end iterator.
        @pre `*this != key_iterator()`.
        */
        key_iterator& increment(std::error_code& ec);

        //! Swaps the contents of `*this` and `other`.
        void swap(key_iterator& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                         class recursive_key_iterator                               //
    //------------------------------------------------------------------------------------//

    //! An iterator to the contents of a registry key and its subkeys.
    /*!
    recursive_key_iterator is an InputIterator that iterates over the key elements of a registry key, and, recursively,
    over the entries of all subkeys. The iteration order is unspecified, except that each entry is visited only once.
    If the recursive_key_iterator is advanced past the last entry of the top-level registry key, it becomes equal to 
    the default-constructed iterator, also known as the end iterator. Two end iterators are always equal, dereferencing
    or incrementing the end iterator is undefined behavior. If an entry is deleted or added to the key tree after the 
    recursive key iterator has been created, it is unspecified whether the change would be observed through the iterator.
    */
    // TODO: key_options ...
    class recursive_key_iterator
    {
        std::vector<key_iterator> m_stack;

    public:
        using value_type =        key_entry;
        using difference_type =   ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::input_iterator_tag;

    public:
        //! Constructs the end iterator.
        recursive_key_iterator() noexcept = default;

        //! Constructs the iterator with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        recursive_key_iterator(const recursive_key_iterator& other) = default;

        /*! \brief
        Constructs the iterator with the contents of `other` using move semantics. `other` is left in a valid but 
        unspecified state. */
        recursive_key_iterator(recursive_key_iterator&& other) noexcept = default;

        //! Constructs a iterator that refers to the first subkey of a registry key specified by `key`. 
        /*!
        If `key` refers to an non-existing registry key, returns the end iterator.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
               std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit recursive_key_iterator(const key& key);

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Constructs an end iterator on error.
        */
        recursive_key_iterator(const key& key, std::error_code& ec);

        //! Constructs a iterator that refers to the first subkey of a registry key specified by `handle`. 
        /*!
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
        `handle.key()`. std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit recursive_key_iterator(const key_handle& handle);

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Constructs an end iterator on error.
        */
        recursive_key_iterator(const key_handle& handle, std::error_code& ec);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        recursive_key_iterator& operator=(const recursive_key_iterator& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, 
        but unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        recursive_key_iterator& operator=(recursive_key_iterator&& other) noexcept = default;

    public:
        // TODO: ...
        bool operator==(const recursive_key_iterator& rhs) const noexcept;

        // TODO: ...
        bool operator!=(const recursive_key_iterator& rhs) const noexcept;

        //! Accesses the pointed-to registry::key.
        /*!
        @pre `*this != recursive_key_iterator()`.
        @return Value of the key referred to by this iterator.
        */
        reference operator*() const;

        //! Accesses the pointed-to registry::key.
        /*!
        @pre `*this != recursive_key_iterator()`.
        @return Pointer to the key referred to by this iterator.
        */
        pointer operator->() const;

        /*! \brief
        Returns the number of keys from the starting key to the currently iterated key, i.e. the current depth of 
        the key hierarchy. */
        /*!
        The starting key has depth of 0, its subkeys have depth 1, etc. 
        @pre `*this != recursive_key_iterator()`.
        */
        int depth() const;

    public:
        //! Calls increment(), then returns `*this`.
        /*!
        @pre `*this != recursive_key_iterator()`.
        @throw registry::registry_error on underlying OS API errors. std::bad_alloc may be thrown if memory allocation 
               fails.
        */
        recursive_key_iterator& operator++();

        //! Makes a copy of `*this`, calls increment(), then returns the copy.
        /*!
        @pre `*this != recursive_key_iterator()`.
        @throw registry::registry_error on underlying OS API errors. std::bad_alloc may be thrown if memory allocation 
               fails.
        */
        recursive_key_iterator operator++(int);

        //! Advances the iterator to the next entry. 
        /*!
        If an error occurs `*this` is becoming equal to the end iterator.
        @pre `*this != recursive_key_iterator()`.
        */
        recursive_key_iterator& increment(std::error_code& ec);

        //! Moves the iterator one level up in the key hierarchy. 
        /*!
        If the parent key is outside key hierarchy that is iterated on (i.e. `depth() == 0`), sets `*this` to an end 
        iterator. 
        @pre `*this != recursive_key_iterator()`.
        */
        void pop();

        //! Swaps the contents of `*this` and `other`.
        void swap(recursive_key_iterator& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                             class value_iterator                                   //
    //------------------------------------------------------------------------------------//

    //! An iterator to the values of a registry key.
    /*!
    value_iterator is an InputIterator that iterates over the values of a registry key. The iteration order is 
    unspecified, except that each entry is visited only once. The default value is not iterated through. If the 
    value_iterator is advanced past the last entry, it becomes equal to the default-constructed iterator, also known 
    as the end iterator. Two end iterators are always equal, dereferencing or incrementing the end iterator is undefined 
    behavior. If an entry is deleted or added to the key tree after the value iterator has been created, it is unspecified
    whether the change would be observed through the iterator. 
    */
    class value_iterator
    {
        uint32_t                              m_idx;
        key_handle                            m_hkey;
        value_entry                           m_entry;
        std::vector<string_type::value_type>  m_buffer;

    public:
        using value_type =        value_entry;
        using difference_type =   ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::input_iterator_tag;

    public:
        //! Constructs the end iterator.
        value_iterator() noexcept = default;

        //! Constructs the iterator with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        value_iterator(const value_iterator& other) = default;

        /*! \brief
        Constructs the iterator with the contents of `other` using move semantics. `other` is left in a valid but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        value_iterator(value_iterator&& other) noexcept = default;

        //! Constructs a iterator that refers to the first value of a key specified by `key`.
        /*!
        If `key` refers to an non-existing registry key, returns the end iterator.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
               std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit value_iterator(const key& key);

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Constructs an end iterator on error.
        */
        value_iterator(const key& key, std::error_code& ec);

        //! Constructs a iterator that refers to the first value of a key specified by `handle`.
        /*!
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
        `handle.key()`. std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit value_iterator(const key_handle& handle);

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Constructs an end iterator on error.
        */
        value_iterator(const key_handle& handle, std::error_code& ec);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        value_iterator& operator=(const value_iterator& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, 
        but unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        value_iterator& operator=(value_iterator&& other) noexcept = default;

    public:
        // TODO: ...
        bool operator==(const value_iterator& rhs) const noexcept;

        // TODO: ...
        bool operator!=(const value_iterator& rhs) const noexcept;

        //! Accesses the pointed-to registry::value_entry.
        /*!
        @pre `*this != value_iterator()`.
        @return Value of the value_entry referred to by this iterator.
        */
        reference operator*() const;

        //! Accesses the pointed-to registry::value_entry.
        /*!
        @pre `*this != value_iterator()`.
        @return Pointer to the value_entry referred to by this iterator.
        */
        pointer operator->() const;

    public:
        //! Calls increment(), then returns `*this`.
        /*!
        @pre `*this != value_iterator()`.
        @throw registry::registry_error on underlying OS API errors. std::bad_alloc may be thrown if memory allocation 
               fails.
        */
        value_iterator& operator++();

        //! Makes a copy of `*this`, calls increment(), then returns the copy.
        /*!
        @pre `*this != value_iterator()`.
        @throw registry::registry_error on underlying OS API errors. std::bad_alloc may be thrown if memory allocation 
               fails.
        */
        value_iterator operator++(int);

        //! Advances the iterator to the next entry.
        /*!
        If an error occurs *this is becoming equal to the end iterator.
        @pre `*this != value_iterator()`.
        */
        value_iterator& increment(std::error_code& ec);

        //! Swaps the contents of `*this` and `other`.
        void swap(value_iterator& other) noexcept;
    };


    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    constexpr access_rights operator&(access_rights lhs, access_rights rhs) noexcept;

    constexpr access_rights operator|(access_rights lhs, access_rights rhs) noexcept;

    constexpr access_rights operator^(access_rights lhs, access_rights rhs) noexcept;

    constexpr access_rights operator~(access_rights lhs) noexcept;

    access_rights& operator&=(access_rights& lhs, access_rights rhs) noexcept;

    access_rights& operator|=(access_rights& lhs, access_rights rhs) noexcept;

    access_rights& operator^=(access_rights& lhs, access_rights rhs) noexcept;

    constexpr key_options operator&(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator|(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator^(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator~(key_options lhs) noexcept;

    key_options& operator&=(key_options& lhs, key_options rhs) noexcept;

    key_options& operator|=(key_options& lhs, key_options rhs) noexcept;

    key_options& operator^=(key_options& lhs, key_options rhs) noexcept;

    constexpr key_info_mask operator&(key_info_mask lhs, key_info_mask rhs) noexcept;

    constexpr key_info_mask operator|(key_info_mask lhs, key_info_mask rhs) noexcept;

    constexpr key_info_mask operator^(key_info_mask lhs, key_info_mask rhs) noexcept;

    constexpr key_info_mask operator~(key_info_mask lhs) noexcept;

    key_info_mask& operator&=(key_info_mask& lhs, key_info_mask rhs) noexcept;

    key_info_mask& operator|=(key_info_mask& lhs, key_info_mask rhs) noexcept;

    key_info_mask& operator^=(key_info_mask& lhs, key_info_mask rhs) noexcept;

    //! Checks whether `lhs` is equal to `rhs`. Equivalent to `lhs.compare(rhs) == 0`.
    bool operator==(const key& lhs, const key& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`. Equivalent to `lhs.compare(rhs) != 0`.
    bool operator!=(const key& lhs, const key& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`. Equivalent to `lhs.compare(rhs) < 0`.
    bool operator<(const key& lhs, const key& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`. Equivalent to `lhs.compare(rhs) > 0`.
    bool operator>(const key& lhs, const key& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`. Equivalent to `lhs.compare(rhs) <= 0`.
    bool operator<=(const key& lhs, const key& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`. Equivalent to `lhs.compare(rhs) >= 0`.
    bool operator>=(const key& lhs, const key& rhs) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key& lhs, key& rhs) noexcept;

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

    //! Checks whether `lhs` is equal to `rhs`.
    bool operator==(const value_entry& lhs, const value_entry& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`.
    bool operator!=(const value_entry& lhs, const value_entry& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`.
    bool operator<(const value_entry& lhs, const value_entry& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`.
    bool operator>(const value_entry& lhs, const value_entry& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`.
    bool operator<=(const value_entry& lhs, const value_entry& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`.
    bool operator>=(const value_entry& lhs, const value_entry& rhs) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(value_entry& lhs, value_entry& rhs) noexcept;

    //! Checks whether `lhs` is equal to `rhs`.
    bool operator==(const key_handle& lhs, const key_handle& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`.
    bool operator!=(const key_handle& lhs, const key_handle& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`.
    bool operator<(const key_handle& lhs, const key_handle& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`.
    bool operator>(const key_handle& lhs, const key_handle& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`.
    bool operator<=(const key_handle& lhs, const key_handle& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`.
    bool operator>=(const key_handle& lhs, const key_handle& rhs) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key_handle& lhs, key_handle& rhs) noexcept;

    //! Returns `it` unchanged.
    const key_iterator& begin(const key_iterator& it) noexcept;

    //! Returns a default-constructed key_iterator, which serves as the end iterator. The argument is ignored.
    key_iterator end(const key_iterator&) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key_iterator& lhs, key_iterator& rhs) noexcept;

    //! Returns `it` unchanged.
    const recursive_key_iterator& begin(const recursive_key_iterator& it) noexcept;

    //! Returns a default-constructed recursive_key_iterator, which serves as the end iterator. The argument is ignored.
    recursive_key_iterator end(const recursive_key_iterator&) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(recursive_key_iterator& lhs, recursive_key_iterator& rhs) noexcept;

    //! Returns `it` unchanged.
    const value_iterator& begin(const value_iterator& it) noexcept;

    //! Returns a default-constructed value_iterator, which serves as the end iterator. The argument is ignored.
    value_iterator end(const value_iterator&) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(value_iterator& lhs, value_iterator& rhs) noexcept;

    //! Opens a registry key and returns a handle to that key. 
    /*!
    @param[in] key - an absolute key specifying the registry key that this function opens.
    @param[in] rights - the access rights for the key to be opened.
    @return a valid key_handle object.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
           std::bad_alloc may be thrown if memory allocation fails.
    */
    key_handle open(const key& key, access_rights rights);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `key_handle()` on error.
    */
    key_handle open(const key& key, access_rights rights, std::error_code& ec);

    //! Check whether a registry key exist. 
    /*!
    @param[in] key - an absolute key specifying the registry key that this function checks the existence of.
    @return `true` if the given key corresponds to an existing registry key, `false` otherwise.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
           std::bad_alloc may be thrown if memory allocation fails.
    */
    bool exists(const key& key);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `false` on error.
    */
    bool exists(const key& key, std::error_code& ec);

    //! Check whether a registry value exists.
    /*!
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the
                            default value.
    @return `true` if the given name corresponds to an existing registry value, `false` otherwise.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key` and
           the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
    */
    bool exists(const key& key, string_view_type value_name);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `false` on error.
    */
    bool exists(const key& key, string_view_type value_name, std::error_code& ec);

    //! Retrieves information about a registry key.
    /*!
    @param[in] key - an absolute key specifying the registry key that this function queries the information about.
    @param[in] mask - a mask specifying which fields of key_id structure are filled out and which aren't.
                      The fields of key_id which aren't filled out will have default-constructed values.
    @return an instance of key_info.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`.
           std::bad_alloc may be thrown if memory allocation fails.
    */
    key_info info(const key& key, key_info_mask mask);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `key_info()` on error.
    */
    key_info info(const key& key, key_info_mask mask, std::error_code& ec);

    //! Reads the content of an existing registry value.
    /*!
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the 
                            default value.
    @return An instance of registry::value.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key` and
           the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
    */
    value read_value(const key& key, string_view_type value_name);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument. 
    /*!
    Returns an default-constructed value on error.
    */
    value read_value(const key& key, string_view_type value_name, std::error_code& ec);

    //! Creates a registry key.
    /*!
    The parent key must already exist. If the key already exists, the function does nothing (this condition is not 
    treated as an error).
    @param[in] key - the key.
    @return `true` if key creation is successful, `false` otherwise.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
           std::bad_alloc may be thrown if memory allocation fails.
    */

    //! Creates a registry key.
    /*!
    If the key already exists, the function has no effect (the returned value is `false`). The function creates 
    all missing keys in the specified path.
    @param[in] key - an absolute key specifying the registry key that this function creates.
    @return `true` if key creation is successful, `false` otherwise.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`.
           std::bad_alloc may be thrown if memory allocation fails.
    */
    bool create_key(const key& key);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument. 
    /*!
    Returns `false` on error.
    */
    bool create_key(const key& key, std::error_code& ec);

    //! Writes an value to an existing registry key.
    /*!
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the
                            default value.
    @param[in] value - the content of the value.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key` and
           the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
    */
    void write_value(const key& key, string_view_type value_name, const value& value);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    void write_value(const key& key, string_view_type value_name, const value& value, std::error_code& ec);

    //! Deletes an registry key.
    /*!
    The key to be deleted must not have subkeys. To delete a key and all its subkeys use `remove_all` function. \n
    @param[in] key - an absolute key specifying the registry key that this function deletes.
    @return `true` if the key was deleted, `false` if it did not exist.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key()`.
           std::bad_alloc may be thrown if memory allocation fails.
    */
    bool remove(const key& key);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `false` on error.
    */
    bool remove(const key& key, std::error_code& ec);

    //! Deletes an registry value.
    /*!
    @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the
                            default value.
    @return `true` if the value was deleted, `false` if it did not exist.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`
           and the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
    */
    bool remove(const key& key, string_view_type value_name);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `false` on error.
    */
    bool remove(const key& key, string_view_type value_name, std::error_code& ec);

    //! Deletes the contents of `key` and the contents of all its subkeys, recursively, then deletes `key` itself as if 
    //! by repeatedly applying remove.
    /*!
    @param[in] key - the key to remove.
    @return the number of keys that were deleted (which may be zero if `key` did not exist to begin with).
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
           std::bad_alloc may be thrown if memory allocation fails.
    */
    std::uintmax_t remove_all(const key& key);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument. 
    /*!
    Returns `static_cast<std::uintmax_t>(-1)` on error.
    */
    std::uintmax_t remove_all(const key& key, std::error_code& ec);

    //! Checks whether two existing keys, refer to the same registry key.
    /*!
    @param[in] key1 - an absolute key specifying the path to the first key.
    @param[in] key2 - an absolute key specifying the path to the second key.
    @return `true` if `key1` and `key2` resolve to the same registry key, else `false`.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key1` and 
           the second key set to `key2`. std::bad_alloc may be thrown if memory allocation fails.
    */
    bool equivalent(const key& key1, const key& key2);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `false` on error.
    */
    bool equivalent(const key& key1, const key& key2, std::error_code& ec);


    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

    inline constexpr access_rights operator&(access_rights lhs, access_rights rhs) noexcept
    { return static_cast<access_rights>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); }

    inline constexpr access_rights operator|(access_rights lhs, access_rights rhs) noexcept
    { return static_cast<access_rights>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); }

    inline constexpr access_rights operator^(access_rights lhs, access_rights rhs) noexcept
    { return static_cast<access_rights>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); }

    inline constexpr access_rights operator~(access_rights lhs) noexcept
    { return static_cast<access_rights>(~static_cast<uint32_t>(lhs)); }

    inline access_rights& operator&=(access_rights& lhs, access_rights rhs) noexcept { return lhs = lhs & rhs; }

    inline access_rights& operator|=(access_rights& lhs, access_rights rhs) noexcept { return lhs = lhs | rhs; }

    inline access_rights& operator^=(access_rights& lhs, access_rights rhs) noexcept { return lhs = lhs ^ rhs; }

    inline constexpr key_options operator&(key_options lhs, key_options rhs) noexcept
    { return static_cast<key_options>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); }

    inline constexpr key_options operator|(key_options lhs, key_options rhs) noexcept
    { return static_cast<key_options>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); }

    inline constexpr key_options operator^(key_options lhs, key_options rhs) noexcept
    { return static_cast<key_options>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); }

    inline constexpr key_options operator~(key_options lhs) noexcept
    { return static_cast<key_options>(~static_cast<uint32_t>(lhs)); }

    inline key_options& operator&=(key_options& lhs, key_options rhs) noexcept { return lhs = lhs & rhs; }

    inline key_options& operator|=(key_options& lhs, key_options rhs) noexcept { return lhs = lhs | rhs; }

    inline key_options& operator^=(key_options& lhs, key_options rhs) noexcept { return lhs = lhs ^ rhs; }

    inline constexpr key_info_mask operator&(key_info_mask lhs, key_info_mask rhs) noexcept
    { return static_cast<key_info_mask>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); }

    inline constexpr key_info_mask operator|(key_info_mask lhs, key_info_mask rhs) noexcept
    { return static_cast<key_info_mask>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); }

    inline constexpr key_info_mask operator^(key_info_mask lhs, key_info_mask rhs) noexcept
    { return static_cast<key_info_mask>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); }

    inline constexpr key_info_mask operator~(key_info_mask lhs) noexcept
    { return static_cast<key_info_mask>(~static_cast<uint32_t>(lhs)); }

    inline key_info_mask& operator&=(key_info_mask& lhs, key_info_mask rhs) noexcept { return lhs = lhs & rhs; }

    inline key_info_mask& operator|=(key_info_mask& lhs, key_info_mask rhs) noexcept { return lhs = lhs | rhs; }

    inline key_info_mask& operator^=(key_info_mask& lhs, key_info_mask rhs) noexcept { return lhs = lhs ^ rhs; }

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

    inline bool operator==(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) == 0; }

    inline bool operator!=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) != 0; }

    inline bool operator<(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) < 0; }

    inline bool operator>(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) > 0; }

    inline bool operator<=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) <= 0; }

    inline bool operator>=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) >= 0; }

    inline void swap(key& lhs, key& rhs) noexcept { lhs.swap(rhs); }

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

    inline bool operator==(const value_entry& lhs, const value_entry& rhs) noexcept 
    { return lhs.key() == rhs.key() && lhs.value_name() == rhs.value_name(); }

    inline bool operator!=(const value_entry& lhs, const value_entry& rhs) noexcept { return !(lhs == rhs); }

    inline bool operator<(const value_entry& lhs, const value_entry& rhs) noexcept 
    { return lhs.key() < rhs.key() || (lhs.key() == rhs.key() && lhs.value_name() < rhs.value_name()); }

    inline bool operator>(const value_entry& lhs, const value_entry& rhs) noexcept 
    { return lhs.key() > rhs.key() || (lhs.key() == rhs.key() && lhs.value_name() > rhs.value_name()); }

    inline bool operator<=(const value_entry& lhs, const value_entry& rhs) noexcept { return !(lhs > rhs); }

    inline bool operator>=(const value_entry& lhs, const value_entry& rhs) noexcept { return !(lhs < rhs); }

    inline void swap(value_entry& lhs, value_entry& rhs) noexcept { lhs.swap(rhs); }

    inline bool operator==(const key_handle& lhs, const key_handle& rhs) noexcept 
    { return lhs.native_handle() == rhs.native_handle(); }

    inline bool operator!=(const key_handle& lhs, const key_handle& rhs) noexcept { return !(lhs == rhs); }

    inline bool operator<(const key_handle& lhs, const key_handle& rhs) noexcept
    { return lhs.native_handle() < rhs.native_handle(); }

    inline bool operator>(const key_handle& lhs, const key_handle& rhs) noexcept
    { return lhs.native_handle() > rhs.native_handle(); }

    inline bool operator<=(const key_handle& lhs, const key_handle& rhs) noexcept { return !(lhs > rhs); }

    inline bool operator>=(const key_handle& lhs, const key_handle& rhs) noexcept { return !(lhs < rhs); }

    inline void swap(key_handle& lhs, key_handle& rhs) noexcept { lhs.swap(rhs); }

    inline const key_iterator& begin(const key_iterator& it) noexcept { return it; }

    inline key_iterator end(const key_iterator&) noexcept { return key_iterator(); }

    inline void swap(key_iterator& lhs, key_iterator& rhs) noexcept { lhs.swap(rhs); }

    inline const recursive_key_iterator& begin(const recursive_key_iterator& it) noexcept { return it; }

    inline recursive_key_iterator end(const recursive_key_iterator&) noexcept { return recursive_key_iterator(); }

    inline void swap(recursive_key_iterator& lhs, recursive_key_iterator& rhs) noexcept { lhs.swap(rhs); }

    inline const value_iterator& begin(const value_iterator& it) noexcept { return it; }

    inline value_iterator end(const value_iterator&) noexcept { return value_iterator(); }

    inline void swap(value_iterator& lhs, value_iterator& rhs) noexcept { lhs.swap(rhs); }
}