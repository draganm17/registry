/** @file */
#pragma once

#include <cstdint>
#include <system_error>

#include <registry/types.h>
#include <registry/value.h>


namespace registry
{
    class key;

    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    //! Creates a registry key.
    /*!
    If the key already exists, the function has no effect (the returned value is `false`). The function creates 
    all missing keys in the specified path.
    @param[in] key - an absolute key specifying the registry key that this function creates.
    @param[out] ec - out-parameter for error reporting.
    @return `true` if key creation is successful, `false` otherwise. The overload that takes `std::error_code&` 
            parameter returns `false` on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key` and the OS error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    bool create_key(const key& key, std::error_code& ec = throws());

    //! Checks whether two existing keys, refer to the same registry key.
    /*!
    @param[in] key1 - an absolute key specifying the path to the first registry key.
    @param[in] key2 - an absolute key specifying the path to the second registry key.
    @param[out] ec - out-parameter for error reporting.
    @return `true` if `key1` and `key2` resolve to the same registry key, `false` otherwise. The overload that 
            takes `std::error_code&` parameter returns `false` on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key1`, the second key set to `key2` and the OS error 
           code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    bool equivalent(const key& key1, const key& key2, std::error_code& ec = throws());

    //! Check whether a registry key exist. 
    /*!
    @param[in] key - an absolute key specifying the registry key that this function checks the existence of.
    @param[out] ec - out-parameter for error reporting.
    @return `true` if the given key corresponds to an existing registry key, `false` otherwise. The overload that
            takes `std::error_code&` parameter returns `false` on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key` and the OS error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    bool exists(const key& key, std::error_code& ec = throws());

    //! Check whether a registry value exists.
    /*!
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                            default value.
    @param[out] ec - out-parameter for error reporting.
    @return `true` if the given name corresponds to an existing registry value, `false` otherwise. The overload that
            takes `std::error_code&` parameter returns `false` on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key`, the value name set to `value_name` and the OS
           error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    bool exists(const key& key, string_view_type value_name, std::error_code& ec = throws());  

    //! Retrieves information about a registry key.
    /*!
    @param[in] key - an absolute key specifying the registry key that this function queries the information about.
    @param[in] mask - a mask specifying which members of key_id are filled out and which aren't. The members which 
                      aren't filled out will be set to `static_cast<uint32_t>(-1)`, except for `last_write_time`, 
                      which default value is `key_time_type::min()`.
    @param[out] ec - out-parameter for error reporting.
    @return An instance of key_info. The overload that takes `std::error_code&` parameter sets all members but 
            `last_write_time` to `static_cast<uint32_t>(-1)` on error, `last_write_time` is set to 
            `key_time_type::min()`.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key` and the OS error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    key_info info(const key& key, key_info_mask mask, std::error_code& ec = throws());

    //! Reads the content of an existing registry value.
    /*!
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the 
                            default value.
    @param[out] ec - out-parameter for error reporting.
    @return An instance of registry::value. The overload that takes `std::error_code&` parameter returns an 
            default-constructed value on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key`, the value name set to `value_name` and the OS
           error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    value read_value(const key& key, string_view_type value_name, std::error_code& ec = throws());

    //! Deletes an registry key.
    /*!
    The key to be deleted must not have subkeys. To delete a key and all its subkeys use `remove_all` function. \n
    Note that a deleted key is not removed until the last handle to it is closed.
    @param[in] key - an absolute key specifying the registry key that this function deletes.
    @param[out] ec - out-parameter for error reporting.
    @return `true` if the key was deleted, `false` if it did not exist. The overload that takes `std::error_code&` 
            parameter returns `false` on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key` and the OS error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    bool remove(const key& key, std::error_code& ec = throws());

    //! Deletes an registry value.
    /*!
    @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                            default value.
    @param[out] ec - out-parameter for error reporting.
    @return `true` if the value was deleted, `false` if it did not exist. The overload that takes `std::error_code&` 
            parameter returns `false` on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key`, the value name set to `value_name` and the OS
           error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    bool remove(const key& key, string_view_type value_name, std::error_code& ec = throws());

    /*! \brief
    Deletes the contents of `key` and the contents of all its subkeys, recursively, then deletes `key` itself as if 
    by repeatedly applying `registry::remove`. */
    /*!
    @param[in] key - an absolute key specifying the registry key that this function deletes.
    @param[out] ec - out-parameter for error reporting.
    @return the number of keys that were deleted (which may be zero if `key` did not exist to begin with). The 
            overload that takes `std::error_code&` parameter returns `static_cast<uintmax_t>(-1)` on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key` and the OS error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    uintmax_t remove_all(const key& key, std::error_code& ec = throws());

    //! Retrieves the information about the size of the registry on the system.
    /*!
    @param[out] ec - out-parameter for error reporting.
    @return an instance of space_info. The overload that takes `std::error_code&` parameter sets all members to 
            `static_cast<uint32_t>(-1)` on error.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the OS error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    space_info space(std::error_code& ec = throws());

    //! Writes an value to an existing registry key.
    /*!
    @param[in] key - an absolute key specifying the location of the value.
    @param[in] value_name - a null-terminated string containing the value name. An empty string correspond to the
                            default value.
    @param[in] value - the content of the value.
    @param[out] ec - out-parameter for error reporting.
    @throw The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
           API errors, constructed with the first key set to `key`, the value name set to `value_name` and the OS
           error code as the error code argument. \n
           `std::bad_alloc` may be thrown by both overloads if memory allocation fails. The overload taking a 
           `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, and executes 
           `ec.clear()` if no errors occur. 
    */
    void write_value(const key& key, string_view_type value_name, const value& value, std::error_code& ec = throws());

} // namespace registry