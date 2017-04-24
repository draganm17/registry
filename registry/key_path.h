/** @file */
#pragma once

#include <cstdint>
#include <iterator>
#include <type_traits>

#include <registry/types.h>


namespace registry
{
    /*! On 64-bit Windows, portions of the registry entries are stored separately for 32-bit application and 64-bit
    applications and mapped into separate logical registry views using the registry redirector and registry reflection,
    because the 64-bit version of an application may use different registry keys and values than the 32-bit version. 
    These flags enable explicit access to the 64-bit registry view and the 32-bit view, respectively. For more 
    information see: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724072 */
    enum class view : uint32_t
    {
        /*! Access a 32-bit key from a 32-bit application or a 64-bit key from a 64-bit application. */
        view_default =  0x00000000,

        /*! Access a 64-bit key from either a 32-bit or 64-bit application. */
        view_32bit =    0x00000200,

        /*! Access a 32-bit key from either a 32-bit or 64-bit application. \n
        Ignored on the 32-bit versions of Windows. */
        view_64bit =    0x00000100
    };

    //------------------------------------------------------------------------------------//
    //                                 class key_path                                     //
    //------------------------------------------------------------------------------------//

    //! Represents a path to a registry key.
    /*!
    Objects of type `registry::key_path` represent paths on the Windows registry. Only syntactic aspects of paths are
    handled: the pathname may represent a non-existing registry key or even one that is not allowed to exist on Windows. \n
    A key path is composed of two parts: `registry::view` and the key name. The latter has the following syntax:
    1. Predefined key identifier (optional): a root registry key (such as `HKEY_LOCAL_MACHINE`).
    2. Zero or more of the following:
        - subkey name:    sequence of characters that aren't key separators.
        - key-separators: the backslash character `\`. If this character is repeated, it is treated as a single key 
                          separator: `HKEY_LOCAL_MACHINE\Software` is the same as `HKEY_LOCAL_MACHINE\\\\Software`.

    The key name can be traversed element-wise via iterators returned by the `begin()` and `end()` functions, which 
    iterates over all subkeys (key separators are skipped). Calling any non-const member function of a key invalidates
    all iterators referring to elements of that object.
    */
    class key_path
    {
    public:
        class iterator;
        using const_iterator = iterator;

    public:
        //! Key separator. Always a backslash.
        static constexpr string_type::value_type separator = string_type::value_type('\\');

    private:
        key_path& append_impl(string_view_type subkey);

    public:
        //! Constructs a path that corresponds to an predefined registry key.
        /*!
        Returns `key_path()` if `id == key_id::unknown`. \n
        The view of the returned path is always equal to `view::view_default`.
        */
        static key_path from_key_id(key_id id);

    public:
        //! Default constructor.
        /*!
        @post `key_name().empty()`.
        @post `key_view() == view::view_default`.
        */
        key_path() noexcept = default;

        //! Constructs the path with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        key_path(const key_path& other) = default;

        /*! \brief
        Constructs the path with the contents of `other` using move semantics. `other` is left in a valid but
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        key_path(key_path&& other) noexcept = default;

        //! Constructs the path from a key name string and a registry view.
        /*!
        @post `key_name() == static_cast<string_type>(name)`.
        @post `key_view() == view`.

        @param[in] name - a key name string.
        @param[in] view - a registry view.
        */
        // TODO: construct the path in a generic format :
        //       - remove all redundant separators
        //       - upcase all characters ???
        key_path(string_view_type name, view view = view::view_default);

        // TODO: ...
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<string_view_type, Source>::value>
        >
        key_path(const Source& name);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        key_path& operator=(const key_path& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key_path& operator=(key_path&& other) noexcept = default;

    public:
        //! Returns the name of the key.
        const string_type& key_name() const noexcept;

        //! Returns the registry view of the key.
        view key_view() const noexcept;

        //! Returns the root path of the path.
        /*!
        If the key name is not empty and its first component identifies a predefined registry key,
        returns `key_path(*begin(), key_view())`. Otherwise, returns `key_path(string_type(), key_view())`.
        */
        key_path root_path() const;

        //! Returns the identifier of the root key.
        /*!
        if `!has_root_path()`, returns `key_id::unknown`.
        */
        key_id root_key_id() const noexcept;

        //! Returns the leaf component of the path.
        /*!
        If the key name is not empty, returns `key_path(*--end(), key_view())`.
        Otherwise, returns `key_path(string_type(), key_view())`.
        */
        key_path leaf_path() const;

        //! Returns the parent of the path.
        /*!
        Returns `key_path(string_type(), key_view())` if `!has_parent_path()`. The resulting path is 
        constructed by appending all  elements in a range `[begin(), --end())` to an path constructed as 
        `key_path(string_type(), key_view())`.
        */
        // TODO: investigate what should be the appropriate behaviour
        key_path parent_path() const;

        //! Returns a path relative to the root path.
        /*!
        The path is composed of every component of `*this` after the root-path. \n
        If the key name is empty, returns `key_path(string_type(), key_view())`.
        */
        key_path relative_path() const;

        //! Checks whether `root_path()` key name is empty.
        bool has_root_path() const noexcept;

        //! Checks whether `leaf_path()` key name is empty.
        bool has_leaf_path() const noexcept;

        //! Checks whether `parent_path()` key name is empty.
        bool has_parent_path() const noexcept;

        //! Checks whether `relative_path()` key name is empty.
        bool has_relative_path() const noexcept;

        //! Checks whether the path is absolute.
        /*!
        An absolute path is a path that unambiguously identifies the location of a registry key without reference to
        an additional starting location. The key name of an absolute path always begins with a predefined key identifier.
        */
        bool is_absolute() const noexcept;

        //! Checks whether the path is relative.
        /*!
        Equivalent to `!is_absolute()`.
        */
        bool is_relative() const noexcept;

        //! Compares paths objects.
        /*!
        - if `key_view() < other.key_view()`, `*this` is less than `other`;
        - otherwise if `key_view() > other.key_view()`, `*this` is greater than `other`;
        - otherwise the key name components are compared lexicographically. The comparison is case-insensitive.

        @return
            A value less than 0 if this path is less than the given path. \n
            A value equal to 0 if this path is equal to the given path.   \n
            A value greater than 0 if this path is greater than the given path.
        */
        int compare(const key_path& other) const noexcept;

        //! Returns an iterator to the first component of the key name.
        /*!
        If the key name is empty, returns `end()`.
        */
        iterator begin() const noexcept;

        /*! \brief
        Returns an iterator one past the last component of the key name. Dereferencing this iterator is undefined 
        behavior. */
        iterator end() const noexcept;

    public:
        //! Replaces the contents of the path.
        /*!
        @post `*this == key_path(name, view)`.

        @param[in] name - a key name string.
        @param[in] view - a registry view.

        @return `*this`.
        */
        key_path& assign(string_view_type name, view view = view::view_default);

        //! Appends elements to the key name.
        /*!
        First, appends the key separator to the key name, except if any of the following conditions is true:
        - the separator would be redundant (the key name already ends with a separator);
        - the key name has no components, i.e. `begin() == end()`;
        - `subkey` is an empty string;
        - `subkey` begins with a key separator.

        Then, appends `subkey` to the key name.

        @param[in] subkey - a string, such as `Source` should be explicitly convertible to `registry::string_view_type`.

        @return `*this`.
        */
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<string_view_type, Source>::value>
        >
        key_path& append(const Source& subkey);

        //! Appends elements to the key name.
        /*!
        First, appends each component of `subkey` name to the key name as if by 
        `for (auto it = subkey.begin(); it != subkey.end(); ++it) append(*it);`. \n
        Then, assigns the key view to `subkey.key_view()`, except if `subkey.key_view() == view::view_default`.

        @return `*this`.
        */
        key_path& append(const key_path& subkey);

        //! Concatenates the key name with `str` without introducing a key separator.
        /*!
        Equivalent to `*this = key_path(key_name() + static_cast<string_type>(str)), key_view())`.

        @return `*this`.
        */
        // TODO: make shure to never introduce a separator, document that
        key_path& concat(string_view_type str);

        //! Removes a single leaf component.
        /*!
        @pre `has_leaf_path()`.
        @return `*this`.
        */
        key_path& remove_leaf_path();

        //! Replaces a single leaf component with `replacement`.
        /*!
        Equivalent to `remove_leaf_path().append(replacement)`.

        @pre `has_leaf_path()`.
        @return `*this`.
        */
        key_path& replace_leaf_path(string_view_type replacement);

        // TODO: ...
        // same as prev. overload but additionally may replace the view
        //key_path& replace_leaf_path(const key_path& replacement);

        //! Swaps the contents of `*this` and `other`.
        void swap(key_path& other) noexcept;

    private:
        view         m_view = view::view_default;
        string_type  m_name;

    };

    //------------------------------------------------------------------------------------//
    //                          class key_path::iterator                                  //
    //------------------------------------------------------------------------------------//

    //! A constant BidirectionalIterator with a value_type of registry::string_view_type.
    class key_path::iterator
    {
        friend class key_path;

        string_view_type  m_value;
        string_view_type  m_key_name_view;

    public:
        using value_type =        string_view_type;
        using difference_type =   std::ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        //! Checks whether `*this` is equal to `rhs`.
        /*!
        Equivalent to `operator*().data() == rhs.operator*().data() && operator*().size() == rhs.operator*().size()`.
        */
        bool operator==(const iterator& rhs) const noexcept;

        //! Checks whether `*this` is not equal to `rhs`.
        /*!
        Equivalent to `!(*this == rhs)`.
        */
        bool operator!=(const iterator& rhs) const noexcept;

        //! Accesses the pointed-to registry::string_view_type.
        /*!
        @pre `*this` is a valid iterator and not the end iterator.
        @return Value of the string_view_type referred to by this iterator.
        */
        reference operator*() const noexcept;

        //! Accesses the pointed-to registry::string_view_type.
        /*!
        @pre `*this` is a valid iterator and not the end iterator.
        @return Pointer to the string_view_type referred to by this iterator.
        */
        pointer operator->() const noexcept;

    public:
        //! Advances the iterator to the next entry, then returns `*this`.
        /*!
        @pre `*this` is a valid iterator and not the end iterator.
        */
        iterator& operator++() noexcept;

        //! Makes a copy of `*this`, calls operator++(), then returns the copy.
        /*!
        @pre `*this` is a valid iterator and not the end iterator.
        */
        iterator operator++(int) noexcept;

        //! Shifts the iterator to the previous entry, then returns `*this`.
        /*!
        @pre `*this` is a valid iterator and not the begin iterator.
        */
        iterator& operator--() noexcept;

        //! Makes a copy of `*this`, calls operator--(), then returns the copy.
        /*!
        @pre `*this` is a valid iterator and not the begin iterator.
        */
        iterator operator--(int) noexcept;

    public:
        //! Swaps the contents of `*this` and `other`.
        void swap(iterator& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    //! Checks whether `lhs` is equal to `rhs`. Equivalent to `lhs.compare(rhs) == 0`.
    bool operator==(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`. Equivalent to `lhs.compare(rhs) != 0`.
    bool operator!=(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`. Equivalent to `lhs.compare(rhs) < 0`.
    bool operator<(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`. Equivalent to `lhs.compare(rhs) > 0`.
    bool operator>(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`. Equivalent to `lhs.compare(rhs) <= 0`.
    bool operator<=(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`. Equivalent to `lhs.compare(rhs) >= 0`.
    bool operator>=(const key_path& lhs, const key_path& rhs) noexcept;

    //! Calculates a hash value for a `key_path` object.
    /*!
    @return A hash value such that if for two paths, `p1 == p2` then `hash_value(p1) == hash_value(p2)`.
    */
    size_t hash_value(const key_path& path) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key_path& lhs, key_path& rhs) noexcept;

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

    template <typename Source, 
              typename = std::enable_if_t<std::is_constructible<string_view_type, Source>::value>
    >
    key_path::key_path(const Source& name) : key_path(string_view_type(name)) { }

    template <typename Source, 
              typename = std::enable_if_t<std::is_constructible<string_view_type, Source>::value>
    >
    inline key_path& key_path::append(const Source& subkey) { return append_impl(subkey); }

    inline bool operator==(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) == 0; }

    inline bool operator!=(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) != 0; }

    inline bool operator<(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) < 0; }

    inline bool operator>(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) > 0; }

    inline bool operator<=(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) <= 0; }

    inline bool operator>=(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) >= 0; }

    inline void swap(key_path& lhs, key_path& rhs) noexcept { lhs.swap(rhs); }

} // namespace registry