/** @file */
#pragma once

#include <cstdint>
#include <exception>
#include <memory>
#include <system_error>
#include <type_traits>

#include <registry/key_path.h>
#include <registry/types.h>
#include <registry/value.h>


namespace registry
{
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

        /*! Required to create a subkey of a registry key. */
        create_sub_key =       0x00000004,

        /*! Required to enumerate the subkeys of a registry key. */
        enumerate_sub_keys =   0x00000008,

        /*! Equivalent to `read`. */
        execute =              0x00020019,

        /*! Required to request change notifications for a registry key or for subkeys of a registry key. */
        notify =               0x00000010,

        /*! Required to query the values of a registry key. */
        query_value =          0x00000001,

        /*! TODO: ... */
        read =                 0x00020019,

        /*! Required to create, delete, or set a registry value. */
        set_value =            0x00000002,

        /*! TODO: ... */
        write =                0x00020006,

        /*! TODO: ... */
        unknown =              0x00000000
    };

    //------------------------------------------------------------------------------------//
    //                                class key_handle                                    //
    //------------------------------------------------------------------------------------//

    //! Represents a handle to an registry key.
    /*!
    `registry::key_handle` is a wrapper around a native registry key handle that retains exclusive ownership of that 
    handle. The managed handle is closed when either of the following happens:
    - the managing `key_handle` object is destroyed;
    - the managing `key_handle` object is assigned another handle via `operator=`.

    A `key_handle` may alternatively own no handle, in which case it is called `empty`.
    */
    class key_handle
    {
    public:
        using native_handle_type = void*;

    private:
        struct close_handle_t { void operator()(void*) const noexcept; };

    public:
        //! Constructs an empty handle.
        /*!
        @post `!is_open()`.
        */
        key_handle() noexcept = default;

        key_handle(const key_handle& other) = delete;

        //! Constructs the handle with the contents of `other` using move semantics.
        /*!
        @post `!other.is_open()`.
        @post `*this` has the original value of `other`.
        */
        key_handle(key_handle&& other) noexcept = default;

        // Constructs a handle to an predefined registry key.
        /*!
        Unless `id == key_id::unknown` the postconditions are the following:
        - `is_open()`.
        - `path() == key_path::from_key_id(id)`.
        - `rights() == access_rights::unknown`.
        - `native_handle() == reinterpret_cast<native_handle_type>(id)`.

        Otherwise: `!is_open()`.
        */
        key_handle(key_id id);

        // Constructs a handle to a registry key specified by `path`.
        /*!
        The overload that takes `std::error_code&` parameter constructs an empty handle on error.
        @post `is_open()`.
        @post `this->path() == path`.
        @post `this->rights() == rights`.
        @param[in] path - an absolute key path specifying the registry key that this function opens.
        @param[in] rights - the access rights for the key to be opened.
        @param[out] ec - out-parameter for error reporting.`
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `path` and the OS error code as the error code 
               argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        key_handle(const key_path& path, access_rights rights, std::error_code& ec = throws());

        // TODO: ...
        ~key_handle() noexcept = default;

        key_handle& operator=(const key_handle& other) = delete;

        //! Replaces the contents of `*this` with those of `other` using move semantics.
        /*!
        @post `!other.is_open()`.
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key_handle& operator=(key_handle&& other) noexcept = default;

    public:
        //! Returns the path to the key this handle is to.
        /*!
        If this handle is empty (i.e. `!is_open`), returns `key_path()`.
        */
        key_path path() const;

        //! Returns the access rights this handle was opened with.
        /*!
        If this handle is empty (i.e. `!is_open`), returns `access_rights::unknown`.
        */
        access_rights rights() const noexcept;

        //! Returns the underlying implementation-defined native handle object suitable for use with WinAPI.
        /*!
        If this handle is empty (i.e. `!is_open`), returns `native_handle_type{}`.
        */
        native_handle_type native_handle() const noexcept;

        //! Returns `true` if this handle is open (i.e. not empty) and `false` otherwise.
        bool is_open() const noexcept;

    public:
        //! Creates a subkey inside the registry key specified by this handle.
        /*!
        If the key already exists, the function opens it. The function creates all missing keys in the specified path. \n
        The calling process must have `access_rights::create_sub_key` access to the key specified by this handle. The 
        access rights the key was opened with does not affect the operation.
        @param[in] path - an relative key path specifying the subkey that this function opens or creates. If the subkey
                          name is an empty string the function will return a new handle to the key specified by this
                          handle.
        @param[in] rights - the access rights for the key to be created.
        @param[out] ec - out-parameter for error reporting.
        @return A pair consisting of an handle to the opened or created key and a `bool` denoting whether the key was
                created. The returned handle is constructed as if by `key_handle(this->path().append(path), rights)`.
                The overload that takes `std::error_code&` parameter returns `std::make_pair(key_handle(), false)` 
                on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the second key path set to `path`
               and the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        std::pair<key_handle, bool> create_key(const key_path& path,
                                               access_rights rights = access_rights::all_access, 
                                               std::error_code& ec = throws()) const;

        /*! \brief 
        Checks whether the registry key specified by this handle and the registry key specified by `path` refer to
        the same registry key. */
        /*!
        The key must have been opened with the `access_rights::query_value` access right.
        @param[in] path - an absolute registry key path.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if `*this` and `path` resolve to the same registry key, `false` otherwise. The overload that 
                takes `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the second key path set `path` 
               and the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        bool equivalent(const key_path& path, std::error_code& ec = throws()) const;

        /*! \brief 
        Checks whether the registry key specified by this handle and the registry key specified by 
        `handle` refer to the same registry key. */
        /*!
        Both keys must have been opened with the `access_rights::query_value` access right.
        @param[in] handle - a handle to an opened registry key.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if `*this` and `handle` resolve to the same registry key, `false` otherwise. The overload that 
                takes `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying
               OS API errors, constructed with the first key path set to `this->path()`, the second key path set to 
               `handle.path()` and the OS  error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        bool equivalent(const key_handle& handle, std::error_code& ec = throws()) const;

        // TODO: exists() for subkeys ???

        //! Check whether the registry key specified by this handle contains the given value.
        /*!
        The key must have been opened with the `access_rights::query_value` access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                                default value.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if the given name corresponds to an existing registry value, `false` otherwise. The overload  
                that takes `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the value name set to `value_name` 
               and the OS error code as the error code argument. \n
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
        @return An instance of key_info. The overload that takes `std::error_code&` parameter sets all members but 
                `last_write_time` to `static_cast<uint32_t>(-1)` on error, `last_write_time` is set to 
                `key_time_type::min()`.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()` and the OS error code as the error
               code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        key_info info(key_info_mask mask = key_info_mask::all, std::error_code& ec = throws()) const;

        //! Opens a subkey of a registry key specified by this handle.
        /*!
        @param[in] path - an relative key path specifying the subkey that this function opens.
        @param[in] rights - the access rights for the key to be opened.
        @param[out] ec - out-parameter for error reporting.
        @return An `key_handle` object constructed as if by `key_handle(this->path().append(path), rights)`.
                The overload  that takes `std::error_code&` parameter returns an empty handle on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the second key path set to `path`
               and the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        // TODO: describe what happens if subkey name is empty ...
        key_handle open(const key_path& path, access_rights rights, std::error_code& ec = throws()) const;

        /*! \brief
        Retrieves the type and data for the specified value name associated with the registry key specified by this 
        handle. /*
        /*!
        The key must have been opened with the `access_rights::query_value` access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the 
                                default value.
        @param[out] ec - out-parameter for error reporting.
        @return An instance of registry::value. The overload that takes `std::error_code&` parameter returns an 
                default-constructed value on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the value name set to `value_name`
               and the OS error code as the error code argument. \n
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
        @param[in] path - an relative key path specifying the subkey that this function deletes.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if the subkey was deleted, `false` if it did not exist. The overload  that takes 
                `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the second key path set `path` and
               the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        bool remove(const key_path& path, std::error_code& ec = throws()) const;

        //! Deletes an registry value from the registry key specified by this handle.
        /*!
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                                default value.
        @param[out] ec - out-parameter for error reporting.
        @return `true` if the value was deleted, `false` if it did not exist. The overload  that takes 
                `std::error_code&` parameter returns `false` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the value name set to `value_name`
               and the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur. 
        */
        bool remove(string_view_type value_name, std::error_code& ec = throws()) const;

        //! Deletes an subkey and all its subkeys, recursively, from the registry key specified by this handle.
        /*!
        The key must have been opened with the `access_rights::enumerate_sub_keys` and `access_rights::query_value` 
        access right. \n
        Note that a deleted key is not removed until the last handle to it is closed.
        @param[in] path - an relative key path specifying the subkey that this function deletes.
        @param[out] ec - out-parameter for error reporting.
        @return The number of keys that were deleted (which may be zero if `path` did not exist to begin with). 
                The overload that takes `std::error_code&` parameter returns `static_cast<uint32_t>(-1)` on error.
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the second key path set `path` and
               the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur.
        */
        uint32_t remove_all(const key_path& path, std::error_code& ec = throws()) const;

        //! Sets the data and type of a specified value under the registry key specified by this handle.
        /*!
        The key must have been opened with the `access_rights::set_value` access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                                default value.
        @param[in] value - the content of the value.
        @param[out] ec - out-parameter for error reporting.`
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()`, the value name set to `value_name`
               and the OS error code as the error code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur. 
        */
        void write_value(string_view_type value_name, const value& value, std::error_code& ec = throws()) const;

    public:
        //! Closes this handle.
        /*!
        If `*this` is an empty handle (i.e. `is_open() == false`), does nothing and does not report an error. \n
        If `*this` is not an empty handle (i.e. `is_open() == true`) and is not a handle to one of the predefined
        registry keys, establishes the postcondition as if by WinAPI `RegCloseKey(native_handle())`. \n
        If an error occurred, the handle is left in an empty state.
        @post `!is_open()`.
        @param[out] ec - out-parameter for error reporting.`
        @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
               API errors, constructed with the first key path set to `this->path()` and the OS error code as the error
               code argument. \n
               `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
               `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
               `ec.clear()` if no errors occur. 
        */
        void close(std::error_code& ec = throws());

        //! Swaps the contents of `*this` and `other`.
        void swap(key_handle& other) noexcept;

    private:
        key_path                               m_path;
        access_rights                          m_rights;
        std::unique_ptr<void, close_handle_t>  m_handle;
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

    //! Calculates a hash value for a `key_handle` object.
    /*!
    @return A hash value such that if for two handles, `h1 == h2` then `hash_value(h1) == hash_value(h2)`.
    */
    size_t hash_value(const key_handle& handle) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key_handle& lhs, key_handle& rhs) noexcept;

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