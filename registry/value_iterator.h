/** @file */
#pragma once

#include <iterator>
#include <memory>
#include <system_error>

#include <registry/key.h>
#include <registry/key_handle.h>
#include <registry/types.h>
#include <registry/value.h>


namespace registry
{
    //------------------------------------------------------------------------------------//
    //                              class value_entry                                     //
    //------------------------------------------------------------------------------------//

    // TODO: ...
    class value_entry
    {
        friend class value_iterator;

    public:
        //! Default constructor.
        /*!
        @post `key() == registry::key()`.
        @post `value_name().empty()`.
        */
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

        //! TODO: ...
        /*!
        @post `this->key() == key`.
        @post `this->value_name() == value_name`.
        */
        value_entry(const key& key, string_view_type value_name);

        //! TODO: ...
        /*!
        @post `this->key() == handle.key()`.
        @post `this->value_name() == value_name`.
        */
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
        //! Returns the key this object was initializes with.
        const key& key() const noexcept;

        //! Returns the value name this object was initializes with.
        const string_type& value_name() const noexcept;

        // TODO: ...
        registry::value value(std::error_code& ec = throws()) const;

    public:
        //! Replaces the contents of the entry.
        /*!
        @post `*this == key_entry(key, value_name)`.
        */
        value_entry& assign(const registry::key& key, string_view_type value_name);

        //! Replaces the contents of the entry.
        /*!
        @post `*this == key_entry(handle, value_name)`.
        */
        value_entry& assign(const registry::key_handle& handle, string_view_type value_name);

        //! Swaps the contents of `*this` and `other`.
        void swap(value_entry& other) noexcept;

    private:
        registry::key    m_key;
        string_type      m_value_name;
        weak_key_handle  m_key_handle;
    };
    
    //------------------------------------------------------------------------------------//
    //                             class value_iterator                                   //
    //------------------------------------------------------------------------------------//

    //! An iterator to the values of a registry key.
    /*!
    value_iterator is an InputIterator that iterates over the values of a registry key. The iteration order is 
    unspecified, except that each entry is visited only once. The default value is not iterated through. If the 
    value_iterator is advanced past the last entry, it becomes equal to the default-constructed iterator, also 
    known as the end iterator. Two end iterators are always equal, dereferencing or incrementing the end iterator is 
    undefined behavior. If an entry is deleted or added to the key tree after the value iterator has been created, it
    is unspecified whether the change would be observed through the iterator. 
    */
    class value_iterator
    {
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
        If `key` refers to an non-existing registry key, returns the end iterator. The overload that takes 
        `std::error_code&` parameter constructs an end iterator on error.
        @param[in] key - TODO: ...
        @param[out] ec - out-parameter for error reporting.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `key` and the OS error code as the error code
               argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        explicit value_iterator(const key& key, std::error_code& ec = throws());

        //! Constructs a iterator that refers to the first value of a key specified by `handle`.
        /*!
        The overload that takes `std::error_code&` parameter constructs an end iterator on error.
        @param[in] handle - a handle to an opened key. The key must have been opened with the 
                             `access_rights::query_value` access right.
        @param[out] ec - out-parameter for error reporting.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `handle.key()` and the OS error code as the error code
               argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        explicit value_iterator(const key_handle& handle, std::error_code& ec = throws());

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
        //! Checks whether `*this` is equal to `rhs`.
        bool operator==(const value_iterator& rhs) const noexcept;

        //! Checks whether `*this` is not equal to `rhs`.
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

    private:
        struct state;
        std::shared_ptr<state> m_state;
    };


    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

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
    const value_iterator& begin(const value_iterator& it) noexcept;

    //! Returns a default-constructed value_iterator, which serves as the end iterator. The argument is ignored.
    value_iterator end(const value_iterator&) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(value_iterator& lhs, value_iterator& rhs) noexcept;

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

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

    inline const value_iterator& begin(const value_iterator& it) noexcept { return it; }

    inline value_iterator end(const value_iterator&) noexcept { return value_iterator(); }

    inline void swap(value_iterator& lhs, value_iterator& rhs) noexcept { lhs.swap(rhs); }

}// namespace registry