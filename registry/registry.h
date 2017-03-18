/*!
## Registry library version 0.91 ##
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
    enum class view       : uint32_t;
    enum class key_id     : uintptr_t;
    enum class value_type : uint32_t;

    class bad_key_name;
    class bad_value_cast;
    class registry_error;
    class key;
    class value;
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

    //! Predefined key identifier. 
    //! See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724836(v=vs.85).aspx
    enum class key_id : uintptr_t 
    {
        classes_root =                 0x80000000,
        current_user =                 0x80000001,
        local_machine =                0x80000002,
        users =                        0x80000003,
        performance_data =             0x80000004,
        current_config =               0x80000005,
        dyn_data =                     0x80000006,
        current_user_local_settings =  0x80000007,
    };

    //! The registry key view. 
    //! See: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724072
    enum class view : uint32_t
    {
        /*! 32-bit registry key view. */
        view_32bit =    0x0200,

        /*! 64-bit registry key view.\n
        ignored on the 32-bit versions of Windows. */
        view_64bit =    0x0100
    };

    //! The type of the registry value. 
    //! See: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724884
    enum class value_type : uint32_t
    {
        none =                 0,
        sz =                   1,
        expand_sz =            2,
        binary =               3,
        dword =                4,
        dword_big_endian =     5,
        link =                 6,
        multi_sz =             7,
        qword =                11
    };
    //TODO: Should I support REG_RESOURCE_LIST, REG_FULL_RESOURCE_DESCRIPTOR and REG_RESOURCE_REQUIREMENTS_LIST types ?
    //      If not, should I specify an 'unknown' type in case such a value is readed into registry::value object ?

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

    //------------------------------------------------------------------------------------//
    //                              class bad_key_name                                    //
    //------------------------------------------------------------------------------------//

    //! Defines a type of object to be thrown by registry::key constructor on failure.
    class bad_key_name
        : public std::exception
    {
    public:
        const char* what() const noexcept override { return "registry::bad_key_name"; }
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
            key_id       m_root;
            string_type  m_name;
        };
    } //\endcond

    //------------------------------------------------------------------------------------//
    //                                   class key                                        //
    //------------------------------------------------------------------------------------//

    //! Represents a registry key.
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
        //! Constructs an empty key.
        /*!
        @post `empty() == true`.
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

        //! Constructs the key from a predefined key identifier and a registry view.
        /*!
        @post `empty() == false`.
        @param[in] root - a predefined key identifier.
        @param[in] view - a registry view.
        */
        key(key_id root, view view = default_view);

        //! Constructs the key from a key name string and a registry view.
        /*!
        @post `empty() == false`.
        @param[in] name - a key name string begining with a valid predefined key identifier.
        @param[in] view - a registry view.
        @throw registry::bad_key_name if `name` is not a valid key name.
        */
        key(string_view_type name, view view = default_view);

        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<string_view_type, Source>::value>
        >
        key(const Source& name) : key(string_view_type(name), default_view) { }

        //! Constructs the key from a predefined key identifier, a subkey string and a registry view.
        /*!
        @post `empty() == false`.
        @param[in] root - a predefined key identifier.
        @param[in] subkey - a subkey string.
        @param[in] view - a registry view.
        */
        key(key_id root, string_view_type subkey, view view = default_view);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        key& operator=(const key& other);

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key& operator=(key&& other) noexcept = default;

    public:
        //! Compares key objects.
        /*!
        Two empty keys are always equal. An empty key is always less than a non-empty one.
        Two non-empty keys are compared using the following rules:
            - if `this->view() < other.view()`, `*this` is less than `other`;
            - otherwise if `this->view() > other.view()`, `*this` is greater than `other`;
            - otherwise keys name components are compared lexicographically. The comparison is case-insensitive.

        @return
            A value less than 0 if the key is less than the given key.\n
            A value equal to 0 if the key is equal to the given key.\n
            A value greater than 0 if the key is greater than the given key.
        */
        int compare(const key& other) const noexcept;

        //! Returns the predefined root key identifier of the key.
        /*!
        @pre `empty() == false`.
        */
        key_id root() const;

        //! Returns the name of the key containing the predefined key identifier string followed by the subkey string.
        /*!
        @pre `empty() == false`.
        */
        string_view_type name() const;

        //! Returns the registry view of the key.
        /*!
        @pre `empty() == false`.
        */
        view view() const;

        //! Returns the subkey string of the key. If the key has no subkey, returns an empty string.
        /*!
        @pre `empty() == false`.
        */
        string_view_type subkey() const;

        //! Checks if the key has a subkey.
        /*!
        @pre `empty() == false`.
        @return `true` if the key has a subkey, `false` otherwise.
        */
        bool has_subkey() const;

        //! Returns the parent of the key. If the key has no parent, returns an empty key.
        /*!
        @pre `empty() == false`.
        */
        key parent_key() const;

        //! Checks if the key has a parent key.
        /*!
        @pre `empty() == false`.
        @return `true` if the key has a parent key, `false` otherwise.
        */
        bool has_parent_key() const;

        //! Checks if the key is empty.
        /*!
        @return `true` if the key is empty, `false` otherwise.
        */
        bool empty() const noexcept;

        /*! \brief
        Returns an iterator to the first component of the key name. If the key is empty, the returned iterator is 
        equal to end(). */
        iterator begin() const noexcept;

        /*! \brief
        Returns an iterator one past the last component of the key name. Dereferencing this iterator is undefined 
        behavior. */
        iterator end() const noexcept;

    public:
        //! Replaces the contents of the key.
        /*!
        @post `*this == key(root, view)`.
        @param[in] root - a predefined key identifier.
        @param[in] view - a registry view.
        @return `*this`.
        */
        key& assign(key_id root, registry::view view = default_view);

        //! Replaces the contents of the key.
        /*!
        @post `*this == key(name, view)`.
        @param[in] name - a key name string begining with a valid predefined key identifier.
        @param[in] view - a registry view.
        @return `*this`.
        @throw registry::bad_key_name if `name` is not a valid key name.
        */
        key& assign(string_view_type name, registry::view view = default_view);

        //! Replaces the contents of the key.
        /*!
        @post `*this == key(root, subkey, view)`.
        @param[in] root - a predefined key identifier.
        @param[in] subkey - a subkey string.
        @param[in] view - a registry view.
        @return `*this`.
        */
        key& assign(key_id root, string_view_type subkey, registry::view view = default_view);

        //! Appends elements to the subkey.
        /*!
        @pre `empty() == false`.
        @return `*this`.
        */
        key& append(string_view_type subkey);
        
        //! Replaces the predefined key identifier component with replacement.
        /*!
        @pre `empty() == false`.
        @post `this->root() == root`.
        @return `*this`.
        */
        key& replace_root(key_id root);
        
        //! Replaces the subkey component with replacement.
        /*!
        @pre `empty() == false`.
        @pre `has_subkey() == true`.
        @post `this->subkey() == subkey`.
        @return `*this`.
        */
        key& replace_subkey(string_view_type subkey);

        //! Replaces the view component with replacement.
        /*!
        @pre `empty() == false`.
        @post `this->view() == view`.
        @return `*this`.
        */
        key& replace_view(registry::view view);

        //! Removes the subkey component.
        /*!
        @pre `empty() == false`.
        @pre `has_subkey() == true`.
        @post `has_subkey() == false`.
        @return `*this`.
        */
        key& remove_subkey();

        //! Clears the key.
        /*!
        @post `empty() == true`.
        */
        void clear() noexcept;

        //! Swaps the contents of `*this` and `other`.
        void swap(key& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                             class key::iterator                                    //
    //------------------------------------------------------------------------------------//

    //! A constant BidirectionalIterator with a value_type of registry::string_view_type.
    class key::iterator
    {
        friend key;

        string_view_type  m_value;
        string_view_type  m_key_string_view;

    public:
        using value_type =        string_view_type;
        using difference_type =   std::ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        iterator() noexcept = default;

        iterator(const iterator&) = default;

        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) = default;

        iterator& operator=(iterator&&) noexcept = default;

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

        template <typename It>
        auto make_string_enumerator(It first, It last) -> decltype(auto)
        {
            return [first = std::move(first), last = std::move(last)](string_view_type &val) mutable -> bool {
                return first != last ? (val = string_view_type(*first++), true) : false;
            };
        }
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

    //\cond HIDDEN_SYMBOLS
    namespace details
    {
        struct value_entry_state
        {
            key          m_key;
            string_type  m_value_name;
        };
    } //\endcond

    //------------------------------------------------------------------------------------//
    //                              class value_entry                                     //
    //------------------------------------------------------------------------------------//

    class value_entry 
        : private details::value_entry_state
    {
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
        struct state;
        std::unique_ptr<state> m_state;

    public:
        using value_type =        key;
        using difference_type =   ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::input_iterator_tag;

    public:
        //! Constructs the end iterator.
        key_iterator() noexcept;

        //! Constructs the iterator with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        key_iterator(const key_iterator& other);

        /*! \brief
        Constructs the iterator with the contents of `other` using move semantics. `other` is left in a valid but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        key_iterator(key_iterator&& other) noexcept;

        //! Constructs a iterator that refers to the first subkey of a key identified by `key`. 
        /*!
        If `key` refers to an non-existing registry key, returns the end iterator. 
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
               std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit key_iterator(const key& key);

        /*!
        Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        Constructs an end iterator on error.
        */
        key_iterator(const key& key, std::error_code& ec);

        ~key_iterator();

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        key_iterator& operator=(const key_iterator& other);

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, 
        but unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key_iterator& operator=(key_iterator&& other) noexcept;

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
    class recursive_key_iterator
    {
        std::vector<key_iterator> m_stack;

    public:
        using value_type =        key;
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

        //! Constructs a iterator that refers to the first subkey of a key identified by `key`. 
        /*!
        If `key` refers to an non-existing registry key, returns the end iterator.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
               std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit recursive_key_iterator(const key& key);

        /*!
        Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        Constructs an end iterator on error.
        */
        recursive_key_iterator(const key& key, std::error_code& ec);

        ~recursive_key_iterator() = default;

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
        struct state;
        std::unique_ptr<state> m_state;

    public:
        using value_type =        value_entry;
        using difference_type =   ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::input_iterator_tag;

    public:
        //! Constructs the end iterator.
        value_iterator() noexcept;

        //! Constructs the iterator with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        value_iterator(const value_iterator& other);

        /*! \brief
        Constructs the iterator with the contents of `other` using move semantics. `other` is left in a valid but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        value_iterator(value_iterator&& other) noexcept;

        //! Constructs a iterator that refers to the first value of a key identified by `key`.
        /*!
        If `key` refers to an non-existing registry key, returns the end iterator.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
               std::bad_alloc may be thrown if memory allocation fails.
        */
        explicit value_iterator(const key& key);

        /*!
        Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        Constructs an end iterator on error.
        */
        value_iterator(const key& key, std::error_code& ec);

        ~value_iterator();

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        value_iterator& operator=(const value_iterator& other);

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, 
        but unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        value_iterator& operator=(value_iterator&& other) noexcept;

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

    //! Check whether a registry key exist. 
    /*!
    @param[in] key - the key.
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
    @param[in] key - the registry key.
    @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the default
               value.
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

    //! Returns the time of the last modification of `key`.
    /*!
    @param[in] key - the registry key.
    @return The time of the last modification of `key`.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
           std::bad_alloc may be thrown if memory allocation fails.
    */
    key_time_type last_write_time(const key& key);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `key_time_type::min()` on error.
    */
    key_time_type last_write_time(const key& key, std::error_code& ec);

    //! Reads the content of an existing registry value.
    /*!
    @param[in] key - the registry key.
    @param[in] name - a null-terminated string containing the value name. An empty name correspond to the default 
               value.
    @return An instance of registry::value.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key` and
           the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
    */
    value read_value(const key& key, string_view_type name);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument. 
    /*!
    Returns an default-constructed value on error.
    */
    value read_value(const key& key, string_view_type name, std::error_code& ec);

    //! Creates a registry key.
    /*!
    The parent key must already exist. If the key already exists, the function does nothing (this condition is not 
    treated as an error).
    @param[in] key - the key.
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

    /*! \brief
    Creates registry keys as if by executing registry::create_key for every element of key that does not already 
    exist. */
    /*!
    @param[in] key - the key.
    @return `true` if keys creation is successful, `false` otherwise.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
           std::bad_alloc may be thrown if memory allocation fails.
    */
    bool create_keys(const key& key);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*! 
    Returns `false` on error.
    */
    bool create_keys(const key& key, std::error_code& ec);

    //! Writes an value to an existing registry key.
    /*!
    @param[in] key - the key to write into.
    @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the default
               value.
    @param[in] value - the value to write.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key` and
           the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
    */
    void write_value(const key& key, string_view_type value_name, const value& value);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    void write_value(const key& key, string_view_type value_name, const value& value, std::error_code& ec);

    //! Deletes an empty (containing no subkeys) registry key.
    /*!
    @param[in] key - the key to remove.
    @return `true` if the key was deleted, `false` if it did not exist.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
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
    @param[in] key - the key containing the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty name correspond to the default
               value.
    @return `true` if the value was deleted, `false` if it did not exist.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key` and
           the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
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
    @param[in] key1 - first registry key.
    @param[in] key2 - second registry key.
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
    inline value& value::assign(multi_sz_value_tag tag, std::initializer_list<T> init) {
        return assign(tag, init.begin(), init.end());
    }

    inline bool operator==(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) == 0; }

    inline bool operator!=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) != 0; }

    inline bool operator<(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) < 0; }

    inline bool operator>(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) > 0; }

    inline bool operator<=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) <= 0; }

    inline bool operator>=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) >= 0; }

    inline void swap(key& lhs, key& rhs) noexcept { lhs.swap(rhs); }

    inline bool operator==(const value& lhs, const value& rhs) noexcept {
        return lhs.type() == rhs.type() && lhs.data() == rhs.data();
    }

    inline bool operator!=(const value& lhs, const value& rhs) noexcept { return !(lhs == rhs); }

    inline bool operator<(const value& lhs, const value& rhs) noexcept {
        return lhs.type() < rhs.type() || (lhs.type() == rhs.type() && lhs.data() < rhs.data());
    }

    inline bool operator>(const value& lhs, const value& rhs) noexcept {
        return lhs.type() > rhs.type() || (lhs.type() == rhs.type() && lhs.data() > rhs.data());
    }

    inline bool operator<=(const value& lhs, const value& rhs) noexcept { return !(lhs > rhs); }

    inline bool operator>=(const value& lhs, const value& rhs) noexcept { return !(lhs < rhs); }

    inline void swap(value& lhs, value& rhs) noexcept { lhs.swap(rhs); }

    inline bool operator==(const value_entry& lhs, const value_entry& rhs) noexcept {
        return lhs.key() == rhs.key() && lhs.value_name() == rhs.value_name();
    }

    inline bool operator!=(const value_entry& lhs, const value_entry& rhs) noexcept { return !(lhs == rhs); }

    inline bool operator<(const value_entry& lhs, const value_entry& rhs) noexcept {
        return lhs.key() < rhs.key() || (lhs.key() == rhs.key() && lhs.value_name() < rhs.value_name());
    }

    inline bool operator>(const value_entry& lhs, const value_entry& rhs) noexcept {
        return lhs.key() > rhs.key() || (lhs.key() == rhs.key() && lhs.value_name() > rhs.value_name());
    }

    inline bool operator<=(const value_entry& lhs, const value_entry& rhs) noexcept { return !(lhs > rhs); }

    inline bool operator>=(const value_entry& lhs, const value_entry& rhs) noexcept { return !(lhs < rhs); }

    inline void swap(value_entry& lhs, value_entry& rhs) noexcept { lhs.swap(rhs); }

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