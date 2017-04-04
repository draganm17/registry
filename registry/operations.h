#pragma once

#include <cstdint>
#include <system_error>

#include <registry/key.h>
#include <registry/types.h>
#include <registry/value.h>


namespace registry
{
    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    //! Creates a registry key.
    /*!
    If the key already exists, the function has no effect (the returned value is `false`). The function creates 
    all missing keys in the specified path.
    @param[in] key - an absolute key specifying the registry key that this function creates.
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

    //! Checks whether two existing keys, refer to the same registry key.
    /*!
    @param[in] key1 - an absolute key specifying the path to the first key.
    @param[in] key2 - an absolute key specifying the path to the second key.
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

    //! Check whether a registry key exist. 
    /*!
    @param[in] key - an absolute key specifying the registry key that this function checks the existence of.
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
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                            default value.
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

    //! Retrieves information about a registry key.
    /*!
    @param[in] key - an absolute key specifying the registry key that this function queries the information about.
    @param[in] mask - a mask specifying which fields of key_id structure are filled out and which aren't.
                      The fields of key_id which aren't filled out will have default-constructed values.
    @return an instance of key_info.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`.
           std::bad_alloc may be thrown if memory allocation fails.
    */
    key_info info(const key& key, key_info_mask mask = key_info_mask::all);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `key_info{}` on error.
    */
    key_info info(const key& key, key_info_mask mask, std::error_code& ec);

    //! Reads the content of an existing registry value.
    /*!
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the 
                            default value.
    @return An instance of registry::value.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key` and
           the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
    */
    value read_value(const key& key, string_view_type value_name);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument. 
    /*!
    Returns an default-constructed value on error.
    */
    value read_value(const key& key, string_view_type value_name, std::error_code& ec);

    //! Deletes an registry key.
    /*!
    The key to be deleted must not have subkeys. To delete a key and all its subkeys use `remove_all` function. \n
    @param[in] key - an absolute key specifying the registry key that this function deletes.
    @return `true` if the key was deleted, `false` if it did not exist.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key()`.
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
    @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                            default value.
    @return `true` if the value was deleted, `false` if it did not exist.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`
           and the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
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

    //! Retrieves the information about the size of the registry on the system.
    /*!
    @return an instance of space_info.
    @throw registry::registry_error on underlying OS API errors, constructed. std::bad_alloc may be thrown if memory 
           allocation fails.
    */
    space_info space();

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    /*!
    Returns `space_info{}` on error.
    */
    space_info space(std::error_code& ec);

    //! Creates a registry key.
    /*!
    The parent key must already exist. If the key already exists, the function does nothing (this condition is not 
    treated as an error).
    @param[in] key - the key.
    @return `true` if key creation is successful, `false` otherwise.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key`. 
           std::bad_alloc may be thrown if memory allocation fails.
    */

    //! Writes an value to an existing registry key.
    /*!
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                            default value.
    @param[in] value - the content of the value.
    @throw registry::registry_error on underlying OS API errors, constructed with the first key set to `key` and
           the value name set to `value_name`. std::bad_alloc may be thrown if memory allocation fails.
    */
    void write_value(const key& key, string_view_type value_name, const value& value);

    //! Same as the previous overload, except underlying OS API errors are reported through the `ec` argument.
    void write_value(const key& key, string_view_type value_name, const value& value, std::error_code& ec);

} // namespace registry