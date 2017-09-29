/** @file */
#pragma once

#include <cstdint>
#include <system_error>

#include <registry/types.h>
#include <registry/value.h>


namespace registry
{
    class key_path;

    //-------------------------------------------------------------------------------------------//
    //                                   NON-MEMBER FUNCTIONS                                    //
    //-------------------------------------------------------------------------------------------//

    //! Creates a registry key.
    /*! The function creates all missing keys in the specified path. If the key already exists, the 
    //  function returns `false` and has no effect.
    //  
    //  @param[in]  path - an absolute key path specifying the registry key that this function creates.
    //
    //  @param[out] ec   - out-parameter for error reporting.
    //  
    //  @return 
    //      `true` if key creation is successful, `false` otherwise. \n
    //      The overload that takes `std::error_code&` parameter returns `false` on error.
    //  
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails,
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    bool create_key(const key_path& path, std::error_code& ec = throws());

    //! Checks whether two paths refer to the same registry key.
    /*!
    //  @param[in]  path1 - an absolute key path specifying the path to the first registry key.
    //
    //  @param[in]  path2 - an absolute key path specifying the path to the second registry key.
    //
    //  @param[out] ec    - out-parameter for error reporting.
    //
    //  @return 
    //      `true` if `path1` and `path2` resolve to the same registry key, `false` otherwise. \n
    //      The overload that takes `std::error_code&` parameter returns `false` on error.
    //
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the first key path set to `path1`, the second key path set to `path2` and the OS error
    //      code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    // TODO: write something about redirection and reflection ???
    bool equivalent(const key_path& path1, const key_path& path2, std::error_code& ec = throws());

    //! Retrieves information about a registry key.
    /*!
    //  @param[in]  path - an absolute key path specifying the registry key that this function queries
    //                     the information about.
    //
    //  @param[in]  mask - a mask specifying which members of key_id are filled out and which aren't.
    //                     The members which aren't filled out will be set to `static_cast<uint32_t>(-1)`,
    //                     except for `last_write_time`, which default value is `key_time_type::min()`.
    //
    //  @param[out] ec -   out-parameter for error reporting.
    //
    //  @return 
    //      An instance of key_info. \n
    //      The overload that takes `std::error_code&` parameter sets all members but `last_write_time`
    //      to `static_cast<uint32_t>(-1)` on error, `last_write_time` is set to `key_time_type::min()`.
    //
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails,
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    key_info info(const key_path& path, key_info_mask mask = key_info_mask::all, std::error_code& ec = throws());

    // TODO: info() for values ???

    //! Check whether a registry key exist. 
    /*!
    //  @param[in]  path - an absolute key path specifying the registry key that this function checks
    //                     the existence of.
    //
    //  @param[out] ec   - out-parameter for error reporting.
    //
    //  @return 
    //      `true` if the given path corresponds to an existing registry key, `false` otherwise. \n
    //      The overload that takes `std::error_code&` parameter returns `false` on error.
    //
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    bool key_exists(const key_path& path, std::error_code& ec = throws());

    //! Retrieves the type and data for the specified value name associated with an registry key.
    /*!
    //  @param[in]  path       - an absolute key path specifying the location of the value.
    //
    //  @param[in]  value_name - a null-terminated string containing the value name. An empty string
    //                           correspond to the default value.
    //
    //  @param[out] ec         - out-parameter for error reporting.
    //
    //  @return 
    //      An instance of registry::value. \n
    //      The overload that takes `std::error_code&` parameter returns an default-constructed value 
    //      on error.
    //  
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the first key path set to `path`, the value name set to `value_name` and the OS error
    //      code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    value read_value(const key_path& path, string_view_type value_name, std::error_code& ec = throws());

    //! Deletes an registry key.
    /*! The key to be deleted must not have subkeys. To delete a key and all its subkeys use 
    //  `remove_keys` function. \n
    //  Note that a deleted key is not removed until the last handle to it is closed.
    //
    //  @param[in]  path - an absolute key path specifying the registry key that this function deletes.
    //
    //  @param[out] ec   - out-parameter for error reporting.
    //
    //  @return 
    //      `true` if the key was deleted, `false` if it did not exist. \n
    //      The overload that takes `std::error_code&` parameter returns `false` on error.
    //  
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    bool remove_key(const key_path& path, std::error_code& ec = throws());

    //! Deletes an registry key and all its subkeys, recursively.
    /*! Note that a deleted key is not removed until the last handle to it is closed.
    //
    //  @param[in]  path - an absolute key path specifying the registry key that this function 
    //                     deletes.
    //
    //  @param[out] ec   - out-parameter for error reporting.
    //
    //  @return 
    //      The number of keys that were deleted (which may be zero if `path` did not exist to begin with). \n
    //      The overload that takes `std::error_code&` parameter returns `static_cast<uint32_t>(-1)` on error.
    //  
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the first key path set to `path` and the OS error code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    uint32_t remove_keys(const key_path& path, std::error_code& ec = throws());

    //! Deletes an registry value.
    /*!
    //  @param[in]  value_name - a null-terminated string containing the value name. An empty string
    //                           correspond to the default value.
    //
    //  @param[out] ec         - out-parameter for error reporting.
    //
    //  @return 
    //      `true` if the value was deleted, `false` if it did not exist. \n
    //      The overload that takes `std::error_code&` parameter returns `false` on error.
    //  
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      constructed with the first key path set to `path`, the value name set to `value_name` and the errors, OS error
    //      code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    bool remove_value(const key_path& path, string_view_type value_name, std::error_code& ec = throws());

    //! Retrieves the information about the size of the registry on the system.
    /*!
    //  @param[out] ec - out-parameter for error reporting.
    //
    //  @return 
    //      An instance of space_info. \n
    //      The overload that takes `std::error_code&` parameter sets all members to 
    //      `static_cast<uint32_t>(-1)` on error.
    //
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the OS error code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    space_info space(std::error_code& ec = throws());

    //! Check whether a registry value exists.
    /*!
    //  @param[in]  path       - an absolute key path specifying the location of the value.
    //
    //  @param[in]  value_name - a null-terminated string containing the value name. An empty string
    //                           correspond to the default value.
    //
    //  @param[out] ec         - out-parameter for error reporting.
    //
    //  @return 
    //      `true` if the given name corresponds to an existing registry value, `false` otherwise. \n
    //      The overload that takes `std::error_code&` parameter returns `false` on error.
    //  
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS API
    //      errors, constructed with the first key path set to `path`, the value name set to `value_name` and the OS error
    //      code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    bool value_exists(const key_path& path, string_view_type value_name, std::error_code& ec = throws());

    //! Sets the data and type of a specified value under a registry key.
    /*!
    //  @param[in]  path       - an absolute key path specifying the location of the value.
    //
    //  @param[in]  value_name - a null-terminated string containing the value name. An empty string
    //                           correspond to the default value.
    //
    //  @param[in]  value      - the content of the value.
    //
    //  @param[out] ec         - out-parameter for error reporting.
    //
    //  @throw 
    //      The overload that does not take a `std::error_code&` parameter throws `registry_error` on underlying OS
    //      API errors, constructed with the first key path set to `path`, the value name set to `value_name` and the
    //      OS error code as the error code argument. \n
    //      The overload taking a `std::error_code&` parameter sets it to the OS API error code if an OS API call fails, 
    //      and executes `ec.clear()` if no errors occur. \n
    //      `std::bad_alloc` may be thrown by both overloads if memory allocation fails.
    */
    void write_value(const key_path& path, string_view_type value_name, const value& value, std::error_code& ec = throws());

} // namespace registry