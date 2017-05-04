/** @file */
#pragma once

#include <iterator>
#include <memory>
#include <system_error>
#include <vector>

#include <registry/details/iterator_utils.h>
#include <registry/key_path.h>
#include <registry/types.h>


namespace registry
{
    class key;

    /*! This type represents available options that control the behavior of the `recursive_key_iterator`. \n
    key_options satisfies the requirements of BitmaskType (which means the bitwise operators `operator&`, 
    `operator|`, `operator^`, `operator~`, `operator&=`, `operator|=`, and `operator^=` are defined for this type) */
    enum class key_options : uint16_t
    {
        /*! (Default) Permission denied is error. */
        none =                    0x0000,

        /*! Skip keys that would otherwise result in permission denied errors. */
        skip_permission_denied =  0x0001
    }; // TODO: rename 'key_options' to something else ???

    //------------------------------------------------------------------------------------//
    //                               class key_entry                                      //
    //------------------------------------------------------------------------------------//

    // TODO: ...
    class key_entry 
    {
        friend class key_iterator;
        friend class recursive_key_iterator;

    public:
        //! Default constructor.
        /*!
        @post `path() == key_path()`.
        */
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

        //! TODO: ...
        /*!
        @post `this->path() == path`.
        */
        explicit key_entry(const key_path& path);

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
        //! Returns the key path this object was initializes with.
        const key_path& path() const noexcept;

        // TODO: ...
        key_info info(key_info_mask mask = key_info_mask::all, std::error_code& ec = throws()) const;

    public:
        //! Replaces the contents of the entry.
        /*!
        @post `*this == key_entry(path)`.
        */
        key_entry& assign(const key_path& path);

        //! Swaps the contents of `*this` and `other`.
        void swap(key_entry& other) noexcept;

    private:
        key_path                               m_path;
        details::possibly_weak_ptr<const key>  m_key_weak_ptr;
    };

    //------------------------------------------------------------------------------------//
    //                              class key_iterator                                    //
    //------------------------------------------------------------------------------------//

    //! An iterator to the contents of the registry key.
    /*!
    key_iterator is an InputIterator that iterates over the key elements of a registry key (but does not visit the 
    subkeys). The iteration order is unspecified, except that each entry is visited only once. If the `key_iterator` is
    advanced past the last entry, it becomes equal to the default-constructed iterator, also known as the end iterator.
    Two end iterators are always equal, dereferencing or incrementing the end iterator is undefined behavior. If an 
    entry is deleted or added to the key tree after the key iterator has been created, it is unspecified whether the 
    change would be observed through the iterator. 
    */
    class key_iterator
    {
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

        //! TODO: ...
        explicit key_iterator(const key& key, std::error_code& ec = throws());

        //! Constructs a iterator that refers to the first subkey of a registry key specified by `path`.
        /*!
        If `path` refers to an non-existing registry key, returns the end iterator and does not report an error.
        The overload that takes `std::error_code&` parameter constructs an end iterator on error.

        @param[in]  path - an absolute key path specifying the registry key that this iterator iterates on.
        @param[out] ec   - out-parameter for error reporting.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails,
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        explicit key_iterator(const key_path& path, std::error_code& ec = throws());

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
        //! Checks whether `*this` is equal to `rhs`.
        bool operator==(const key_iterator& rhs) const noexcept;

        //! Checks whether `*this` is not equal to `rhs`.
        bool operator!=(const key_iterator& rhs) const noexcept;

        //! Accesses the pointed-to registry::key_entry.
        /*!
        @pre `*this != key_iterator()`.
        @return Value of the key_entry referred to by this iterator.
        */
        reference operator*() const;

        //! Accesses the pointed-to registry::key_entry.
        /*!
        @pre `*this != key_iterator()`.
        @return Pointer to the key_entry referred to by this iterator.
        */
        pointer operator->() const;

    public:
        //! Calls `increment()`, then returns `*this`.
        /*!
        @pre `*this != key_iterator()`.
        @throws  Any exception thrown by `increment()`.
        */
        key_iterator& operator++();

        //! Makes a copy of `*this`, calls `increment()`, then returns the copy.
        /*!
        @pre `*this != key_iterator()`.
        @throws  Any exception thrown by `increment()`.
        */
        key_iterator operator++(int);

        //! Advances the iterator to the next entry.
        /*!
        If an error occured, `*this` is set to an end iterator, regardless of whether any error is reported by 
        exception or error code.
        @pre `*this != key_iterator()`.
        */
        key_iterator& increment(std::error_code& ec);

        //! Swaps the contents of `*this` and `other`.
        void swap(key_iterator& other) noexcept;

    private:
        struct state;
        std::shared_ptr<state> m_state;
    };

    //------------------------------------------------------------------------------------//
    //                         class recursive_key_iterator                               //
    //------------------------------------------------------------------------------------//

    //! An iterator to the contents of a registry key and its subkeys.
    /*!
    recursive_key_iterator is an InputIterator that iterates over the key elements of a registry key, and, recursively,
    over the entries of all subkeys. The iteration order is unspecified, except that each entry is visited only once.
    If the `recursive_key_iterator` is advanced past the last entry of the top-level registry key, it becomes equal to 
    the default-constructed iterator, also known as the end iterator. Two end iterators are always equal, dereferencing
    or incrementing the end iterator is undefined behavior. If an entry is deleted or added to the key tree after the 
    recursive key iterator has been created, it is unspecified whether the change would be observed through the iterator.
    */
    // TODO: recursion_pending
    class recursive_key_iterator
    {
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

        //! Constructs a iterator that refers to the first subkey of a registry key specified by `path`.
        /*!
        Calls `recursive_key_iterator(path, key_options::none, ec)`.
        */
        explicit recursive_key_iterator(const key_path& path, std::error_code& ec = throws());

        //! Constructs a iterator that refers to the first subkey of a registry key specified by `path`. 
        /*!
        If `path` refers to an non-existing registry key, returns the end iterator and does not report an error. \n
        if `(options & key_options::skip_permission_denied) != key_options::none` and construction encounters an error
        indicating that permission to access `path` is denied, constructs the end iterator and does not report an error. \n
        The overload that takes  `std::error_code&` parameter constructs an end iterator on error.

        @post `this->options() == options`.

        @param[in]  path    - an absolute key path specifying the registry key that this iterator iterates on.
        @param[in]  options - specify iteration options.
        @param[out] ec      - out-parameter for error reporting.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        recursive_key_iterator(const key_path& path, key_options options, std::error_code& ec = throws());

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
        //! Checks whether `*this` is equal to `rhs`.
        bool operator==(const recursive_key_iterator& rhs) const noexcept;

        //! Checks whether `*this` is not equal to `rhs`.
        bool operator!=(const recursive_key_iterator& rhs) const noexcept;

        //! Accesses the pointed-to registry::key_entry.
        /*!
        @pre `*this != recursive_key_iterator()`.
        @return Value of the key_entry referred to by this iterator.
        */
        reference operator*() const;

        //! Accesses the pointed-to registry::key_entry.
        /*!
        @pre `*this != recursive_key_iterator()`.
        @return Pointer to the key_entry referred to by this iterator.
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

        //! Returns the options that affect the iteration.
        /*!
        The options can only be supplied when constructing the key iterator. If the options argument was not 
        supplied, returns `key_options::none`.

        @pre `*this != recursive_key_iterator()`.
        */
        key_options options() const;

    public:
        //! Calls `increment()`, then returns `*this`.
        /*!
        @pre `*this != recursive_key_iterator()`.
        @throws Any exception thrown by `increment()`.
        */
        recursive_key_iterator& operator++();

        //! Makes a copy of `*this`, calls `increment()`, then returns the copy.
        /*!
        @pre `*this != recursive_key_iterator()`.
        @throws Any exception thrown by `increment()`.
        */
        recursive_key_iterator operator++(int);

        //! Advances the iterator to the next entry. 
        /*!
        If there are no more entries left in the currently iterated key, the iteration is resumed over the parent 
        key. The process is repeated if the parent key has no sibling entries that can to be iterated on. If the 
        parent of the key hierarchy that has been recursively iterated on is reached (there are no candidate entries
        at `depth() == 0`), `*this` is set to an end iterator. Otherwise, `*this` is iterated. \n
        If an error occured, `*this` is set to an end iterator, regardless of whether any error is reported by 
        exception or error code.

        @pre `*this != recursive_key_iterator()`.
        */
        recursive_key_iterator& increment(std::error_code& ec);

        //! Moves the iterator one level up in the key hierarchy. 
        /*!
        If `depth() == 0`, set `*this` to `recursive_key_iterator()`. Otherwise, cease iteration of the key currently
        being iterated over, and continue iteration over the parent key. \n
        If an error occured, `*this` is set to an end iterator, regardless of whether any error is reported by 
        exception or error code.

        @pre `*this != recursive_key_iterator()`.

        @param[out] ec - out-parameter for error reporting.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, the OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        void pop(std::error_code& ec = throws());

        //! Swaps the contents of `*this` and `other`.
        void swap(recursive_key_iterator& other) noexcept;

    private:
        std::vector<key_iterator>  m_stack;
        key_options                m_options;
    };

    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    constexpr key_options operator&(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator|(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator^(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator~(key_options lhs) noexcept;

    key_options& operator&=(key_options& lhs, key_options rhs) noexcept;

    key_options& operator|=(key_options& lhs, key_options rhs) noexcept;

    key_options& operator^=(key_options& lhs, key_options rhs) noexcept;

    //! Checks whether `lhs` is equal to `rhs`.
    bool operator==(const key_entry& lhs, const key_entry& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`.
    bool operator!=(const key_entry& lhs, const key_entry& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`.
    bool operator<(const key_entry& lhs, const key_entry& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`.
    bool operator>(const key_entry& lhs, const key_entry& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`.
    bool operator<=(const key_entry& lhs, const key_entry& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`.
    bool operator>=(const key_entry& lhs, const key_entry& rhs) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key_entry& lhs, key_entry& rhs) noexcept;

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

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

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

    inline bool operator==(const key_entry& lhs, const key_entry& rhs) noexcept { return lhs.path() == rhs.path(); }

    inline bool operator!=(const key_entry& lhs, const key_entry& rhs) noexcept { return !(lhs == rhs); }

    inline bool operator<(const key_entry& lhs, const key_entry& rhs) noexcept { return lhs.path() < rhs.path(); }

    inline bool operator>(const key_entry& lhs, const key_entry& rhs) noexcept { return lhs.path() > rhs.path(); }

    inline bool operator<=(const key_entry& lhs, const key_entry& rhs) noexcept { return !(lhs > rhs); }

    inline bool operator>=(const key_entry& lhs, const key_entry& rhs) noexcept { return !(lhs < rhs); }

    inline void swap(key_entry& lhs, key_entry& rhs) noexcept { lhs.swap(rhs); }

    inline const key_iterator& begin(const key_iterator& it) noexcept { return it; }

    inline key_iterator end(const key_iterator&) noexcept { return key_iterator(); }

    inline void swap(key_iterator& lhs, key_iterator& rhs) noexcept { lhs.swap(rhs); }

    inline const recursive_key_iterator& begin(const recursive_key_iterator& it) noexcept { return it; }

    inline recursive_key_iterator end(const recursive_key_iterator&) noexcept { return recursive_key_iterator(); }

    inline void swap(recursive_key_iterator& lhs, recursive_key_iterator& rhs) noexcept { lhs.swap(rhs); }

} // namespace registry