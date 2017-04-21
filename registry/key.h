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

    // TODO: ...
    struct open_only_tag { };

    // TODO: ...
    struct open_or_create_tag { };

    //------------------------------------------------------------------------------------//
    //                                   class key                                        //
    //------------------------------------------------------------------------------------//

    //! Represents a registry key.
    /*!
    TODO: rewrite this ...
    `registry::key_handle` is a wrapper around a native registry key handle that retains exclusive ownership of that 
    handle. The managed handle is closed when either of the following happens:
    - the managing `key_handle` object is destroyed;
    - the managing `key_handle` object is assigned another handle via `operator=`.

    A `key_handle` may alternatively own no handle, in which case it is called `empty`.
    */
    // TODO: remove 'const' modifier from some methods ???
    class key
    {
    public:
        using native_handle_type = void*;

    private:
        struct close_handle_t { void operator()(void*) const noexcept; };

    public:
        //! Constructs an `key` object which does not represent a registry key.
        /*!
        @post `!is_open()`.
        */
        key() noexcept = default;

        key(const key& other) = delete;

        //! Constructs the key with the contents of `other` using move semantics.
        /*!
        @post `!other.is_open()`.
        @post `*this` has the original value of `other`.
        */
        key(key&& other) noexcept = default;

        //! Constructs a `key` object and associates it with a predefined registry key.
        /*!
        Unless `id == key_id::unknown` the postconditions are the following:
        - `is_open()`.
        - `path() == key_path::from_key_id(id)`.
        - `rights() == access_rights::unknown`.
        - `native_handle() == reinterpret_cast<native_handle_type>(id)`. // TODO: expose this ???

        Otherwise: `!is_open()`.

        @param[in] id - TODO: ...
        */
        key(key_id id);

        //! Opens a registry key and associates it with `*his`.
        /*!
        The overload that takes `std::error_code&` parameter constructs an `key` object which does not represent a 
        registry key on error.

        @post `is_open()`.
        @post `this->path() == path`.
        @post `this->rights() == rights`.

        @param[in]  tag    - dispatching tag.
        @param[in]  path   - an absolute key path specifying the registry key that this function opens.
        @param[in]  rights - the access rights for the key to be opened.
        @param[out] ec     - out-parameter for error reporting.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        key(open_only_tag tag, const key_path& path, access_rights rights, std::error_code& ec = throws());


        // TODO: ...
        key(open_or_create_tag tag, const key_path& path, access_rights rights, std::error_code& ec = throws());

        //! Opens or creates a registry key and associates it with `*his`.
        /*!
        The overload that takes `std::error_code&` parameter constructs an `key` object which does not represent a 
        registry key on error.

        @post `is_open()`.
        @post `this->path() == path`.
        @post `this->rights() == rights`.

        @param[in]  tag         - dispatching tag.
        @param[in]  path        - an absolute key path specifying the registry key that this function opens or creates.
        @param[in]  rights      - the access rights for the key to be created.
        @param[out] was_created - receives `true` if the key was created and `false` if it already existed.
        @param[out] ec          - out-parameter for error reporting.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        key(open_or_create_tag tag, const key_path& path, 
            access_rights rights, bool& was_created, std::error_code& ec = throws());

        // TODO: ...
        ~key() noexcept = default;

        key& operator=(const key& other) = delete;

        //! Replaces the contents of `*this` with those of `other` using move semantics.
        /*!
        @post `!other.is_open()`.
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key& operator=(key&& other) noexcept = default;

    public:
        //! Returns the path of the registry key identified by `*this`.
        /*!
        If there is no registry key associated, default constructed `key_path` is returned.
        */
        key_path path() const;

        //! Returns the access rights the key was opened with.
        /*!
        If there is no registry key associated, `access_rights::unknown` is returned.
        */
        access_rights rights() const noexcept;

        //! Returns the underlying implementation-defined native key handle object suitable for use with WinAPI.
        native_handle_type native_handle() const noexcept;

        //! Checks whether the key is open.
        /*!
        @return `true` if the key is open (i.e. `*this` identifies a registry key), `false` otherwise.
        */
        bool is_open() const noexcept;

    public:
        //! Creates a subkey in the registry key identified by `*this`.
        /*!
        The function creates all missing keys in the specified path. If the key already exists, the function opens it. \n
        The calling process must have `access_rights::create_sub_key` access to the key identified by `*this`. The access
        rights the key was opened with does not affect the operation.

        @param[in]  path   - an key path specifying the subkey that this function opens or creates. If the subkey name
                             is an empty string the function will return a new handle to the key specified by this handle.
        @param[in]  rights - the access rights for the key to be created.
        @param[out] ec     - out-parameter for error reporting.

        @return 
            A pair consisting of a `key` object identifying the opened or created registry key and a `bool` denoting
            whether the key was created. The `key` is constructed as if by `key(this->path().append(path), rights)`. \n
            The overload that takes `std::error_code&` parameter returns `std::make_pair(key(), false)` on error.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
            API errors, constructed with the first key path set to `this->path()`, the second key path set to `path`
            and the OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        std::pair<key, bool> create_key(const key_path& path,
                                        access_rights rights = access_rights::all_access, 
                                        std::error_code& ec = throws()) const;

        /*! \brief 
        Checks whether the registry key identified by `*this` and the registry key identified by `path` refer to the
        same registry key. */
        /*!
        The key must have been opened with the `access_rights::query_value` access right.

        @param[in]  path - an absolute registry key path.
        @param[out] ec   - out-parameter for error reporting.

        @return 
            `true` if `*this` and `path` resolve to the same registry key, `false` otherwise. \n
             The overload that takes `std::error_code&` parameter returns `false` on error.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the second key path set `path` and the OS
            error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        bool equivalent(const key_path& path, std::error_code& ec = throws()) const;

        /*! \brief 
        Checks whether the registry key identified by `*this` and the registry key identified by `key` refer to
        the same registry key. */
        /*!
        Both keys must have been opened with the `access_rights::query_value` access right.

        @param[in]  key - an opened registry key.
        @param[out] ec  - out-parameter for error reporting.

        @return 
            `true` if `*this` and `key` identifies the same registry key, `false` otherwise. \n
            The overload that takes `std::error_code&` parameter returns `false` on error.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the second key path set to `key.path()`
            and the OS  error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        bool equivalent(const key& key, std::error_code& ec = throws()) const;

        //! Retrieves information about the registry key identified by `*this`.
        /*!
        The key must have been opened with the `access_rights::query_value` access right.

        @param[in]  mask - a mask specifying which members of key_id are filled out and which aren't. The members which
                           aren't filled out will be set to `static_cast<uint32_t>(-1)`, except for `last_write_time`,
                           which default value is `key_time_type::min()`.
        @param[out] ec   - out-parameter for error reporting.

        @return 
            An instance of key_info. \n
            The overload that takes `std::error_code&` parameter sets all members but `last_write_time`
            to `static_cast<uint32_t>(-1)` on error, `last_write_time` is set to `key_time_type::min()`.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()` and the OS error code as the error code
            argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        key_info info(key_info_mask mask = key_info_mask::all, std::error_code& ec = throws()) const;

        //! Check whether the registry key identified by `*this` contains the given subkey.
        /*!
        @param[in]  path - an key path specifying the subkey that this function checks the existence of.
        @param[out] ec   - out-parameter for error reporting.

        @return 
            `true` if the given path corresponds to an existing registry key, `false` otherwise. \n
            The overload that takes `std::error_code&` parameter returns `false` on error.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
            API errors, constructed with the first key path set to `this->path()`, the second key path set to `path`
            and the OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        bool key_exists(const key_path& path, std::error_code& ec = throws()) const;

        //! Opens a subkey of a registry key identified by `*this`.
        /*!
        @param[in]  path   - an key path specifying the subkey that this function opens.
        @param[in]  rights - the access rights for the key to be opened.
        @param[out] ec     - out-parameter for error reporting.

        @return 
            An `registry::key` object constructed as if by `key(this->path().append(path), rights)`. \n
            The overload  that takes `std::error_code&` parameter returns an default-constructed value on error.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the second key path set to `path` and the
            OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        // TODO: describe what happens if subkey name is empty ...
        key open_key(const key_path& path, access_rights rights, std::error_code& ec = throws()) const;

        /*! \brief
        // TODO: rewrite that bit ...
        Retrieves the type and data for the specified value name associated with the registry key identified by `*this`. /*
        /*!
        The key must have been opened with the `access_rights::query_value` access right.

        @param[in]  value_name - a null-terminated string containing the value name. An empty string correspond to the 
                                 default value.
        @param[out] ec         - out-parameter for error reporting.

        @return 
            An instance of registry::value. \n
            The overload that takes `std::error_code&` parameter returns an default-constructed value on error.
        
        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the value name set to `value_name`and the
            OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        value read_value(string_view_type value_name, std::error_code& ec = throws()) const;

        //! Deletes an subkey from the registry key identified by `*this`.
        /*!
        The subkey to be deleted must not have subkeys. To delete a key and all its subkeys use `remove_keys` function. \n
        The access rights of this key do not affect the delete operation. \n
        Note that a deleted key is not removed until the last handle to it is closed. //TODO: remove or rewrite that line

        @param[in]  path - an key path specifying the subkey that this function deletes.
        @param[out] ec   - out-parameter for error reporting.

        @return 
            `true` if the subkey was deleted, `false` if it did not exist. \n
            The overload  that takes `std::error_code&` parameter returns `false` on error.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the second key path set `path` and the OS
            error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        bool remove_key(const key_path& path, std::error_code& ec = throws()) const;

        //! Deletes an subkey and all its subkeys, recursively, from the registry key identified by `*this`.
        /*!
        The key must have been opened with the `access_rights::enumerate_sub_keys` and `access_rights::query_value` 
        access right. \n
        Note that a deleted key is not removed until the last handle to it is closed. //TODO: remove or rewrite that line

        @param[in]  path - an key path specifying the subkey that this function deletes.
        @param[out] ec   - out-parameter for error reporting.

        @return 
            The number of keys that were deleted (which may be zero if `path` did not exist to begin with). \n
            The overload that takes `std::error_code&` parameter returns `static_cast<uint32_t>(-1)` on error.
        
        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the second key path set `path` and the OS
            error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        uint32_t remove_keys(const key_path& path, std::error_code& ec = throws()) const;

        //! Deletes an registry value from the registry key identified by `*this`.
        /*!
        @param[in]  value_name - a null-terminated string containing the value name. An empty string correspond to the
                                 default value.
        @param[out] ec         - out-parameter for error reporting.

        @return 
            `true` if the value was deleted, `false` if it did not exist. \n
            The overload  that takes `std::error_code&` parameter returns `false` on error.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the value name set to `value_name` and the
            OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        bool remove_value(string_view_type value_name, std::error_code& ec = throws()) const;

        //! Check whether the registry key identified by `*this` contains the given value.
        /*!
        The key must have been opened with the `access_rights::query_value` access right.

        @param[in]  value_name - a null-terminated string containing the value name. An empty string correspond to the
                                 default value.
        @param[out] ec         - out-parameter for error reporting.

        @return 
            `true` if the given name corresponds to an existing registry value, `false` otherwise. \n
            The overload that takes `std::error_code&` parameter returns `false` on error.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the value name set to `value_name` and the
            OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        bool value_exists(string_view_type value_name, std::error_code& ec = throws()) const;

        //! Sets the data and type of a specified value under the registry key identified by `*this`.
        /*!
        The key must have been opened with the `access_rights::set_value` access right.

        @param[in]  value_name - a null-terminated string containing the value name. An empty string correspond to the
                                 default value.
        @param[in]  value      - the content of the value.
        @param[out] ec         - out-parameter for error reporting.

        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()`, the value name set to `value_name` and the
            OS error code as the error code argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        void write_value(string_view_type value_name, const value& value, std::error_code& ec = throws()) const;

    public:
        //! Closes the key.
        /*!
        If there is no registry key associated, does nothing and does not report an error. Otherwise, closes the
        registry key identified by `*this`. \n
        If an error occured, the associated registry key is closed, regardless of whether any error is reported by
        exception or error code.

        @post `!is_open()`.
        
        @param[out] ec - out-parameter for error reporting.`
        
        @throw 
            The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
            errors, constructed with the first key path set to `this->path()` and the OS error code as the error code
            argument. \n
            The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails,
            and executes `ec.clear()` if no errors occur. \n
            `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
        */
        void close(std::error_code& ec = throws());

        //! Swaps the contents of `*this` and `other`.
        void swap(key& other) noexcept;

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

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key& lhs, key& rhs) noexcept;

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

    inline void swap(key& lhs, key& rhs) noexcept { lhs.swap(rhs); }

} // namespace registry