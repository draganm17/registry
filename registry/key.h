#pragma once

#include <cstdint>
#include <iterator>

#include <registry/types.h>


namespace registry
{
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
    // TODO: ...
    */
    class key 
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
        //! Constructs an key object from a given predefined key identifier.
        /*!
        The name of the returned key is equal to the string representation of the predefined key identifier, the key 
        view is equal to `default_view`. The returned key is an absolute key.
        */
        static key from_key_id(key_id id);

    public:
        //! Default constructor.
        /*!
        Equivalent to `key(string_type())`.
        */
        key() noexcept = default;

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
        @post `this->name() == static_cast<string_type>(name)`.
        @post `this->view() == view`.
        @param[in] name - a key name string.
        @param[in] view - a registry view.
        */
        key(string_view_type name, view view = default_view);

        // TODO: ...
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<string_view_type, Source>::value>
        >
        key(const Source& name, view view = default_view) : key(string_view_type(name), view) { }

        // TODO: ...
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
        //! Returns the name of the key.
        const string_type& name() const noexcept;

        //! Returns the registry view of the key.
        view view() const noexcept;

        //! Returns the root component of the key.
        /*!
        Equivalent to `has_root_key() ? key(*begin(), view()) : key(string_type(), view())`.
        */
        key root_key() const;

        //! Returns the leaf component of the key.
        /*!
        Equivalent to `has_leaf_key() ? key(*--end(), view()) : key(string_type(), view())`.
        */
        key leaf_key() const;

        //! Returns the parent of the key.
        /*!
        Equivalent to `has_parent_key() ? key(begin(), --end(), view()) : key(string_type(), view())`.
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
        Equivalent to `has_root_key() && ++begin() != end()`.
        */
        bool has_parent_key() const noexcept;

        //! Checks whether the key is absolute.
        /*!
        An absolute key is a key that unambiguously identifies the location of a registry key. The name of such key
        should begin with a predefined key identifier. \n
        For example:
        - "Software\Microsoft" is an relative key.
        - "\HKEY_LOCAL_MACHINE\Software\Microsoft" is an relative key (note the slash at the begining);
        - "HKEY_LOCAL_MACHINE\Software\Microsoft" is an absolute key;
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
        Returns an iterator to the first component of the key name. If the key name has no components, the returned
        iterator is equal to end(). */
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
        - the key name has no components, i.e. `begin() == end()`;
        - `subkey` is an empty string;
        - `subkey` begins with a key separator.

        Then, appends `subkey` to the key name.
        @return `*this`.
        */
        key& append(string_view_type subkey);

        //! Concatenates the key name with `subkey` without introducing a key separator.
        /*!
        Equivalent to `*this = key(name() + static_cast<string_type>(subkey)), view())`.
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

    private:
        registry::view  m_view = default_view;
        string_type     m_name;

    };

    //------------------------------------------------------------------------------------//
    //                             class key::iterator                                    //
    //------------------------------------------------------------------------------------//

    //! A constant BidirectionalIterator with a value_type of registry::string_view_type.
    class key::iterator
    {
        friend class key;

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

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

    inline bool operator==(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) == 0; }

    inline bool operator!=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) != 0; }

    inline bool operator<(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) < 0; }

    inline bool operator>(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) > 0; }

    inline bool operator<=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) <= 0; }

    inline bool operator>=(const key& lhs, const key& rhs) noexcept { return lhs.compare(rhs) >= 0; }

    inline void swap(key& lhs, key& rhs) noexcept { lhs.swap(rhs); }

} // namespace registry