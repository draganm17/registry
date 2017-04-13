/** @file */
#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <system_error>
#include <type_traits>

#include <registry/key.h>
#include <registry/types.h>
#include <registry/value.h>


namespace registry
{
    //------------------------------------------------------------------------------------//
    //                           class bad_weak_key_handle                                //
    //------------------------------------------------------------------------------------//

    /*! \brief
    Defines a type of the object thrown by the constructors of `registry::key_handle` that take
    `registry::weak_key_handle` as the argument, when the `registry::weak_key_handle` is already expired. */
    class bad_weak_key_handle
        : public std::exception
    {
    public:
        const char* what() const noexcept override { return "registry::bad_weak_key_handle"; }
    };

    //------------------------------------------------------------------------------------//
    //                                class key_handle                                    //
    //------------------------------------------------------------------------------------//

    //! Represents a handle to an registry key.
    /*!
    `registry::key_handle` is a wrapper around a native key handle that retains shared ownership of that handle. 
    Several `key_handle` objects may own the same key handle. The object is destroyed and its handle is closed 
    when either of the following happens:
    - the last remaining `key_handle` owning the key handle is destroyed;
    - the last remaining `key_handle` owning the key handle is assigned another handle via `operator=`.

    A `key_handle` may also own no handle, in which case it is called `invalid`.
    */
    // TODO: describe the internal implementation ???
    class key_handle
    {
        friend class weak_key_handle;

        struct state;
        std::shared_ptr<state> m_state;

    public:
        using native_handle_type = std::underlying_type_t<key_id>;

    public:
        //! Default constructor.
        /*!
        @post `!valid()`.
        */
        constexpr key_handle() noexcept = default;

        /*! \brief
        Constructs a `key_handle` which shares ownership of the handle managed by `other`. If `other` manages no
        handle, `*this` manages no handle too. */
        key_handle(const key_handle& other) noexcept = default;

        //! Constructs the handle with the contents of `other` using move semantics.
        /*!
        @post `!other.valid()`.
        @post `*this` has the original value of `other`.
        */
        key_handle(key_handle&& other) noexcept = default;

        //! Constructs a `key_handle` which shares ownership of the handle managed by `handle`.
        /*!
        @throw `registry::bad_weak_key_handle` if `handle.expired()`.
        */
        key_handle(const weak_key_handle& handle);

        // Constructs a handle to an prdefined registry key.
        /*!
        Unless `id == key_id::unknown` the postconditions are the following:
        - `valid()`.
        - `key() == registry::key::from_key_id(id)`.
        - `rights() == access_rights::unknown`.
        - `native_handle() == static_cast<native_handle_type>(id)`.

        Otherwise: `!valid()`.
        */
        key_handle(key_id id);

        // TODO: ...
        /*!
        Unless `handle == native_handle_type{}` the postconditions are the following:
        - `valid()`.
        - `this->key() == key`.
        - `this->rights() == rights`.
        - `this->native_handle() == handle`.

        Otherwise: `!valid()`.
        */
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
        @post `!other.valid()`.
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key_handle& operator=(key_handle&& other) noexcept = default;

    public:
        //! Returns the key this handle was initialized with.
        /*!
        @return An value of type `registry::key`. If `!valid()` returns `registry::key()`.
        */
        registry::key key() const;

        //! Returns the access rights this handle was initialized with.
        /*!
        @return An value of type `registry::access_rights`. If `!valid()` returns `access_rights::unknown`.
        */
        access_rights rights() const noexcept;

        //! Returns the underlying implementation-defined native handle object suitable for use with WinAPI.
        /*!
        @return An value of type `native_handle_type`. If `!valid()` returns `native_handle_type{}`.
        */
        native_handle_type native_handle() const noexcept;

        // TODO: ...
        bool valid() const noexcept;

    public:
        //! Creates a subkey inside the registry key specified by this handle.
        /*!
        If the key already exists, the function opens it. The function creates all missing keys in the specified path. \n
        The calling process must have `access_rights::create_sub_key` access to the key specified by this handle. The 
        access rights the key was opened with does not affect the operation.
        @param[in] subkey - an relative key specifying the subkey that this function opens or creates. If the subkey
                            name is an empty string the function will return a new handle to the key specified by this
                            handle.
        @param[in] rights - the access rights for the key to be created.
        @param[out] ec - out-parameter for error reporting.
        @return a pair consisting of an handle to the opened or created key and a `bool` denoting whether the key was
                created. The overload that takes `std::error_code&` parameter returns 
                `std::make_pair(key_handle(), false)` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()`, the second key set `subkey` and the OS 
               error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        std::pair<key_handle, bool> create_key(const registry::key& subkey, 
                                               access_rights rights = access_rights::all_access, 
                                               std::error_code& ec = throws()) const;

        /*! \brief 
        Checks whether the registry key specified by this handle and the registry key specified by 
        `key` refer to the same registry key. */
        /*!
        The key must have been opened with the `access_rights::query_value` access right.
        @param[in] key - an absolute registry key.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if `*this` and `key` resolve to the same registry key, `false` otherwise. The overload that 
                takes `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()`, the second key set `key` and the OS 
               error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        bool equivalent(const registry::key& key, std::error_code& ec = throws()) const;

        /*! \brief 
        Checks whether the registry key specified by this handle and the registry key specified by 
        `handle` refer to the same registry key. */
        /*!
        Both keys must have been opened with the `access_rights::query_value` access right.
        @param[in] handle - a handle to an opened registry key.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if `*this` and `handle` resolve to the same registry key, `false` otherwise. The overload that 
                takes `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()`, the second key set `handle.key()` and 
               the OS  error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        bool equivalent(const key_handle& handle, std::error_code& ec = throws()) const;

        //! Check whether the registry key specified by this handle contains the given value.
        /*!
        The key must have been opened with the `access_rights::query_value` access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                                default value.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if the given name corresponds to an existing registry value, `false` otherwise. The overload  
                that takes `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()`, the value name set to `value_name` and
               the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur. 
        */
        bool exists(string_view_type value_name, std::error_code& ec = throws()) const;

        //! Retrieves information about the registry key specified by this handle.
        /*!
        The key must have been opened with the `access_rights::query_value` access right.
        @param[in] mask - a mask specifying which members of key_id are filled out and which aren't. The members which
                          aren't filled out will be set to `static_cast<uint32_t>(-1)`, except for `last_write_time`,
                          which default value is `key_time_type::min()`.
        @param[out] ec - out-parameter for error reporting.
        @return an instance of key_info. The overload that takes `std::error_code&` parameter sets all members but 
                `last_write_time` to `static_cast<uint32_t>(-1)` on error, `last_write_time` is set to 
                `key_time_type::min()`.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()` and the OS error code as the error code 
               argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        key_info info(key_info_mask mask = key_info_mask::all, std::error_code& ec = throws()) const;

        //! Opens a subkey of a registry key specified by this handle
        /*!
        @param[in] key - an relative key specifying the subkey that this function opens.
        @param[in] rights - the access rights for the key to be opened.
        @param[out] ec - out-parameter for error reporting.
        @return a valid `key_handle` object, which `key()` method will return `this->key().append(subkey)` and 
                `rights()` method will return `rights`. The overload  that takes `std::error_code&` parameter returns 
                an  default-constructed (not valid) handle on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `key()`, the second key set to `subkey` and the OS 
               error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        // TODO: describe what happens if subkey name is empty ...
        key_handle open(const registry::key& subkey, access_rights rights, std::error_code& ec = throws()) const;

        //! Reads the content of an registry value contained inside the registry key specified by this handle.
        /*!
        The key must have been opened with the `access_rights::query_value` access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the 
                                default value.
        @param[out] ec - out-parameter for error reporting.
        @return An instance of registry::value. The overload that takes `std::error_code&` parameter returns an 
                default-constructed value on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()`, the value name set to `value_name` and
               the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur. 
        */
        value read_value(string_view_type value_name, std::error_code& ec = throws()) const;

        //! Deletes an subkey from the registry key specified by this handle.
        /*!
        The subkey to be deleted must not have subkeys. To delete a key and all its subkeys use `remove_all` function. \n
        The access rights of this key do not affect the delete operation. \n
        Note that a deleted key is not removed until the last handle to it is closed.
        @param[in] subkey - an relative key specifying the subkey that this function deletes.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if the subkey was deleted, `false` if it did not exist. The overload  that takes 
                `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()`, the second key set `subkey` and the OS 
               error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        bool remove(const registry::key& subkey, std::error_code& ec = throws()) const;

        //! Deletes an registry value from the registry key specified by this handle.
        /*!
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                                default value.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if the value was deleted, `false` if it did not exist. The overload  that takes 
                `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()`, the value name set to `value_name` and
               the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur. 
        */
        bool remove(string_view_type value_name, std::error_code& ec = throws()) const;

        uintmax_t remove_all_inside(const registry::key& subkey, std::error_code& ec) const;

        // TODO: ...
        uintmax_t remove_all(const registry::key& subkey, std::error_code& ec = throws()) const;

        //! Writes an value to the registry key specified by this handle.
        /*!
        The key must have been opened with the `access_rights::set_value` access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                                default value.
        @param[in] value - the content of the value.
        @param[out] ec - out-parameter for error reporting.`
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key set to `this->key()`, the value name set to `value_name` and
               the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur. 
        */
        void write_value(string_view_type value_name, const value& value, std::error_code& ec = throws()) const;

    public:
        //! Swaps the contents of `*this` and `other`.
        void swap(key_handle& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                            class weak_key_handle                                   //
    //------------------------------------------------------------------------------------//

    //! Represents a weak reference to a registry key handle managed by `registry::key_handle`.
    /*!
    `registry::weak_key_handle` is a wrapper around a native key handle that holds a non-owning ("weak") reference to
    a key handle that is managed by `registry::key_handle`. It must be converted to `registry::key_handle` in order to
    access the referenced handle.
    */
    // TODO: describe the internal umplementation ???
    class weak_key_handle
    {
        friend class key_handle;

        std::weak_ptr<key_handle::state> m_state;

    public:
        //! Default constructor. Constructs an invalid `weak_key_handle`.
        /*!
        @post `expired()`.
        */
        constexpr weak_key_handle() noexcept = default;

        /*! \brief
        Constructs a `weak_key_handle` which shares ownership of the handle managed by `other`. If `other` manages no
        handle, `*this` manages no handle too. */
        weak_key_handle(const weak_key_handle& other) noexcept = default;

        //! Constructs the handle with the contents of `other` using move semantics.
        /*!
        @post `other.expired()`.
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
        @post `other.expired()`.
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        weak_key_handle& operator=(weak_key_handle&& other) noexcept = default;

        // TODO: ...
        weak_key_handle& operator=(const key_handle& other) noexcept;

    public:
        //! Checks whether the referenced handle was already closed.
        bool expired() const noexcept;

        //! Creates a `key_handle` that manages the referenced handle. 
        /*!
        If there is no managed handle, i.e. `*this` is invalid, then the returned key_handle also is invalid.
        */
        key_handle lock() const noexcept;

    public:
        //! Swaps the contents of `*this` and `other`.
        void swap(weak_key_handle& other) noexcept;
    };

    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

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

    //! Opens a registry key and returns a handle to that key. 
    /*!
    @param[in] key - an absolute key specifying the registry key that this function opens.
    @param[in] rights - the access rights for the key to be opened.
    @param[out] ec - out-parameter for error reporting.`
    @return a valid `key_handle` object. The overload  that takes `std::error_code&` parameter returns an 
            default-constructed (not valid) handle on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key` and the OS error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur.
    */
    key_handle open(const key& key, access_rights rights, std::error_code& ec = throws());

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

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

} // namespace registry