/** @file */
#pragma once

#include <chrono>
#include <cstdint>
#include <system_error>

// TODO: rename types.h to common.h  ???


namespace registry
{
    //! TODO: ...
    using key_time_type = std::chrono::time_point<std::chrono::system_clock>;

    /*! Windows defines a set of predefined registry keys. These keys are entry points to the registry hierarchy.
    Each such key is also associated with an key handle, which is always open. For more information see: 
    https://msdn.microsoft.com/en-us/library/windows/desktop/ms724836 */
    enum class key_id : uintptr_t 
    {
        /*! Identifies the registry key HKEY_CLASSES_ROOT. \n
        Registry entries subordinate to this key define types (or classes) of documents and the properties 
        associated with those types. */
        classes_root =                 0x80000000,

        /*! Identifies the registry key HKEY_CURRENT_USER. \n
        Registry entries subordinate to this key define the preferences of the current user. */
        current_user =                 0x80000001,

        /*! Identifies the registry key HKEY_LOCAL_MACHINE. \n
        Registry entries subordinate to this key define the physical state of the computer, including data about
        the bus type, system memory, and installed hardware and software. */
        local_machine =                0x80000002,

        /*! Identifies the registry key HKEY_USERS. \n
        Registry entries subordinate to this key define the default user configuration for new users on the local
        computer and the user configuration for the current user. */
        users =                        0x80000003,

        /*! Identifies the registry key HKEY_PERFORMANCE_DATA. \n
        Registry entries subordinate to this key allow you to access performance data. */
        performance_data =             0x80000004,

        /*! Identifies the registry key HKEY_PERFORMANCE_TEXT. \n
        Registry entries subordinate to this key reference the text strings that describe counters in US English. */
        performance_text =             0x80000050,

        /*! Identifies the registry key HKEY_PERFORMANCE_NLSTEXT. \n
        Registry entries subordinate to this key reference the text strings that describe counters in the local
        language of the area in which the computer system is running. */
        performance_nlstext =          0x80000060,

        /*! Identifies the registry key HKEY_CURRENT_CONFIG. \n
        Contains information about the current hardware profile of the local computer system. */
        current_config =               0x80000005,

        /*! Identifies the registry key HKEY_CURRENT_USER_LOCAL_SETTINGS. \n
        Registry entries subordinate to this key define preferences of the current user that are local to the
        machine. */
        current_user_local_settings =  0x80000007,

        //* Unknown key identifier. */
        unknown =                      0x00000000
    };

    /*! TODO: ...
    key_info_mask satisfies the requirements of BitmaskType (which means the bitwise operators `operator&`, 
    `operator|`, `operator^`, `operator~`, `operator&=`, `operator|=`, and `operator^=` are defined for this type) */
    enum class key_info_mask : uint16_t  // TODO: rename to key_info_filter ???
    {
        /*! Request nothing. */
        none =                      0x0000,

        /*! Request the number of subkeys that are contained by the key. */
        read_subkeys =              0x0001,

        /*! Request the number of values that are associated with the key. */
        read_values =               0x0002,

        /*! Request the size of the key's subkey with the longest name. */
        read_max_subkey_size =      0x0004,

        /*! Request the size of the key's longest value name. */
        read_max_value_name_size =  0x0008,

        /*! Request the size of the longest data component among the key's values. */
        read_max_value_data_size =  0x0010,

        /*! Request the last time that the key or any of its value entries is modified. */
        read_last_write_time =      0x0020,

        /*! Request all fields values. */
        all =                       0x003F
    };

    //! Defines a type of object that stores information about a registry key.
    struct key_info
    {
        /*! The number of subkeys that are contained by the key. */
        uint32_t       subkeys;

        /*! The number of values that are associated with the key. */
        uint32_t       values;

        /*!  The size of the key's subkey with the longest name, in characters, not including the terminating 
        null character. */
        uint32_t       max_subkey_size;

        /*! The size of the key's longest value name, in characters, not including the terminating null character. */
        uint32_t       max_value_name_size;

        /*! The size of the longest data component among the key's values, in bytes. */
        uint32_t       max_value_data_size;

        /*! The last time that the key or any of its value entries is modified. */
        key_time_type  last_write_time;
    };

    //! Defines a type of object that stores information about the size of the registry on the system.
    struct space_info
    {
        /*! The maximum size that the registry is allowed to attain on this system, in bytes. */
        uint32_t  capacity;

        /*! The current size of the registry, in bytes. */
        uint32_t  size;
    };

    //\cond HIDDEN_SYMBOLS
    namespace detail { inline constexpr std::error_code* throws() noexcept { return nullptr; } }
    //\endcond

    // TODO: description ...
    inline constexpr std::error_code& throws() noexcept { return *detail::throws(); }


    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    constexpr key_info_mask operator&(key_info_mask lhs, key_info_mask rhs) noexcept;

    constexpr key_info_mask operator|(key_info_mask lhs, key_info_mask rhs) noexcept;

    constexpr key_info_mask operator^(key_info_mask lhs, key_info_mask rhs) noexcept;

    constexpr key_info_mask operator~(key_info_mask lhs) noexcept;

    key_info_mask& operator&=(key_info_mask& lhs, key_info_mask rhs) noexcept;

    key_info_mask& operator|=(key_info_mask& lhs, key_info_mask rhs) noexcept;

    key_info_mask& operator^=(key_info_mask& lhs, key_info_mask rhs) noexcept;


    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

    inline constexpr key_info_mask operator&(key_info_mask lhs, key_info_mask rhs) noexcept
    { return static_cast<key_info_mask>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); }

    inline constexpr key_info_mask operator|(key_info_mask lhs, key_info_mask rhs) noexcept
    { return static_cast<key_info_mask>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); }

    inline constexpr key_info_mask operator^(key_info_mask lhs, key_info_mask rhs) noexcept
    { return static_cast<key_info_mask>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); }

    inline constexpr key_info_mask operator~(key_info_mask lhs) noexcept
    { return static_cast<key_info_mask>(~static_cast<uint32_t>(lhs)); }

    inline key_info_mask& operator&=(key_info_mask& lhs, key_info_mask rhs) noexcept { return lhs = lhs & rhs; }

    inline key_info_mask& operator|=(key_info_mask& lhs, key_info_mask rhs) noexcept { return lhs = lhs | rhs; }

    inline key_info_mask& operator^=(key_info_mask& lhs, key_info_mask rhs) noexcept { return lhs = lhs ^ rhs; }

} // namespace registry