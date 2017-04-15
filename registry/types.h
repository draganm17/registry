/** @file */
#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#if _HAS_CXX17
#include <string_view>
#endif
#include <system_error>
#include <vector>

#if !_HAS_CXX17
#include <boost/utility/string_view.hpp> //awaiting for C++17 std::string_view
#endif


namespace registry
{
#if defined(_UNICODE)
    using string_type =      std::wstring;
#if _HAS_CXX17
    using string_view_type = std::wstring_view;
#else
    using string_view_type = boost::wstring_view;
#endif
#else
    using string_type =      std::string;
#if _HAS_CXX17
    using string_view_type = std::string_view;
#else
    using string_view_type = boost::string_view;
#endif
#endif
    using byte_array_type =      std::vector<uint8_t>;
#if _HAS_CXX17
    using byte_array_view_type = std::basic_string_view<uint8_t>;
    //TODO: replace with std::array_view<uint8_t> when (if ever) available
#else
    using byte_array_view_type = boost::basic_string_view<uint8_t>;
    //TODO: replace with std::array_view<uint8_t> when (if ever) available
#endif
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

    /*! TODO: ...
    key_info_mask satisfies the requirements of BitmaskType (which means the bitwise operators `operator&`, 
    `operator|`, `operator^`, `operator~`, `operator&=`, `operator|=`, and `operator^=` are defined for this type) */
    enum class key_info_mask : uint16_t
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

    constexpr access_rights operator&(access_rights lhs, access_rights rhs) noexcept;

    constexpr access_rights operator|(access_rights lhs, access_rights rhs) noexcept;

    constexpr access_rights operator^(access_rights lhs, access_rights rhs) noexcept;

    constexpr access_rights operator~(access_rights lhs) noexcept;

    access_rights& operator&=(access_rights& lhs, access_rights rhs) noexcept;

    access_rights& operator|=(access_rights& lhs, access_rights rhs) noexcept;

    access_rights& operator^=(access_rights& lhs, access_rights rhs) noexcept;

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