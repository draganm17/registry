#pragma once

#include <cstdint>
#include <iterator>
#include <memory>
#include <system_error>

#include <registry/key.h>
#include <registry/key_handle.h>
#include <registry/types.h>


namespace registry
{
    //------------------------------------------------------------------------------------//
    //                               class key_entry                                      //
    //------------------------------------------------------------------------------------//

    class key_entry 
    {
        friend class key_iterator;
        friend class recursive_key_iterator;

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
        explicit key_entry(const key& key);

        // TODO: ...
        explicit key_entry(const key_handle& handle);

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
        //! Returns the key that was stored in the key entry object.
        const key& key() const noexcept;

        // TODO: ...
        key_info info(key_info_mask mask = key_info_mask::all) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `key_info{}` on error.
        */
        key_info info(key_info_mask mask, std::error_code& ec) const;

    public:
        // TODO: ...
        key_entry& assign(const registry::key& key);

        // TODO: ...
        key_entry& assign(const registry::key_handle& handle);

        //! Swaps the contents of `*this` and `other`.
        void swap(key_entry& other) noexcept;

    private:
        registry::key    m_key;
        weak_key_handle  m_key_handle;
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
    If the recursive_key_iterator is advanced past the last entry of the top-level registry key, it becomes equal to 
    the default-constructed iterator, also known as the end iterator. Two end iterators are always equal, dereferencing
    or incrementing the end iterator is undefined behavior. If an entry is deleted or added to the key tree after the 
    recursive key iterator has been created, it is unspecified whether the change would be observed through the iterator.
    */
    // TODO: key_options ...
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

    private:
        std::vector<key_iterator> m_stack;
    };

    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

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

    inline bool operator==(const key_entry& lhs, const key_entry& rhs) noexcept { return lhs.key() == rhs.key(); }

    inline bool operator!=(const key_entry& lhs, const key_entry& rhs) noexcept { return !(lhs == rhs); }

    inline bool operator<(const key_entry& lhs, const key_entry& rhs) noexcept { return lhs.key() < rhs.key(); }

    inline bool operator>(const key_entry& lhs, const key_entry& rhs) noexcept { return lhs.key() > rhs.key(); }

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