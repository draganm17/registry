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
    Defines a type of the object thrown by the constructors of registry::key_handle that take
    registry::weak_key_handle as the argument, when the registry::weak_key_handle is already expired. */
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
    registry::key_handle is a wrapper around a native key handle that retains shared ownership of that handle. Several 
    key_handle objects may own the same key handle. The object is destroyed and its handle is closed when either of the
    following happens:
    - the last remaining key_handle owning the key handle is destroyed;
    - the last remaining key_handle owning the key handle is assigned another handle via operator=.

    A key_handle may also own no handle, in which case it is called `invalid`.
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
        Constructs a key_handle which shares ownership of the handle managed by `other`. If `other` manages no
        handle, `*this` manages no handle too. */
        key_handle(const key_handle& other) noexcept = default;

        //! Constructs the handle with the contents of `other` using move semantics.
        /*!
        @post `!other.valid()`.
        @post `*this` has the original value of `other`.
        */
        key_handle(key_handle&& other) noexcept = default;

        //! Constructs a key_handle which shares ownership of the handle managed by `handle`.
        /*!
        @throw registry::bad_weak_key_handle if `handle.expired()`.
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
        @return An value of type registry::key. If `!valid()` returns `registry::key()`.
        */
        registry::key key() const;

        //! Returns the access rights this handle was initialized with.
        /*!
        @return An value of type registry::access_rights. If `!valid()` returns `access_rights::unknown`.
        */
        access_rights rights() const noexcept;

        //! Returns the underlying implementation-defined native handle object suitable for use with WinAPI.
        /*!
        @return An value of type native_handle_type. If `!valid()` returns `native_handle_type{}`.
        */
        native_handle_type native_handle() const noexcept;

        // TODO: ...
        bool valid() const noexcept;

    public:
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

        //! Check whether the registry key specified by this handle contains the given value.
        /*!
        The key must have been opened with the access_rights::query_value access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                                default value.
        @return `true` if the given name corresponds to an existing registry value, `false` otherwise.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
               `this->key()` and the value name set to `value_name`. std::bad_alloc may be thrown if memory 
               allocation fails.
        */
        bool exists(string_view_type value_name) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Returns `false` on error.
        */
        bool exists(string_view_type value_name, std::error_code& ec) const;

        //! Retrieves information about the registry key specified by this handle.
        /*!
        The key must have been opened with the access_rights::query_value access right.
        @param[in] mask - a mask specifying which members of key_id are filled out and which aren't. The members which 
                          aren't filled out will be set to `static_cast<uint32_t>(-1)`, except for `last_write_time`, 
                          which default value is `key_time_type::min()`.
        @return an instance of key_info.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to
               `this->key()`. std::bad_alloc may be thrown if memory allocation fails.
        */
        key_info info(key_info_mask mask = key_info_mask::all) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        /*!
        Sets all but `last_write_time` members of the returned value to `static_cast<uint32_t>(-1)` on error.
        `last_write_time` member is set to `key_time_type::min()`.
        */
        key_info info(key_info_mask mask, std::error_code& ec) const;

        //! Reads the content of an registry value contained inside the registry key specified by this handle.
        /*!
        The key must have been opened with the access_rights::query_value access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the 
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
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
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

        //! Writes an value to the registry key specified by this handle.
        /*!
        The key must have been opened with the access_rights::set_value access right.
        @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                                default value.
        @param[in] value - the content of the value.
        @throw registry::registry_error on underlying OS API errors, constructed with the first key set to 
               `this->key()` and the value name set to `value_name`. std::bad_alloc may be thrown if memory 
               allocation fails.
        */
        void write_value(string_view_type value_name, const value& value) const;

        //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
        void write_value(string_view_type value_name, const value& value, std::error_code& ec) const;

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
        @post `expired()`.
        */
        constexpr weak_key_handle() noexcept = default;

        /*! \brief
        Constructs a weak_key_handle which shares ownership of the handle managed by `other`. If `other` manages no 
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