/** @file */
#pragma once

#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
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
    class key;

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

    /*! On 64-bit Windows, portions of the registry entries are stored separately for 32-bit application and 64-bit
    applications and mapped into separate logical registry views using the registry redirector and registry reflection,
    because the 64-bit version of an application may use different registry keys and values than the 32-bit version. 
    These flags enable explicit access to the 64-bit registry view and the 32-bit view, respectively. For more 
    information see: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724072 */
    enum class view : uint32_t
    {
        /*! Access a 64-bit key from either a 32-bit or 64-bit application */
        view_32bit =    0x0200,

        /*! Access a 32-bit key from either a 32-bit or 64-bit application.
        Ignored on the 32-bit versions of Windows. */
        view_64bit =    0x0100
    };

    /*! A registry value can store data in various formats. When you store data under a registry value, you can
    specify one of the following values to indicate the type of data being stored. For more information see: 
    https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724884 */
    enum class value_type : uint32_t
    {
        /*! No defined value type. */
        none =                 0,

        /*! A null-terminated string. */
        sz =                   1,
        
        /*! A null-terminated string that contains unexpanded references to environment variables 
        (for example, "%PATH%"). */
        expand_sz =            2,

        /*! Binary data in any form. */
        binary =               3,
        
        /*! A 32-bit number. */
        dword =                4,
        
        /*! A 32-bit number in big-endian format. */
        dword_big_endian =     5,
        
        /*! A null-terminated string that contains the target path of a symbolic link. */
        link =                 6,
        
        /*! A sequence of null-terminated strings, terminated by an empty string (\0). */
        multi_sz =             7,
        
        /*! A 64-bit number. */
        qword =                11
    };
    //TODO: Should I support REG_RESOURCE_LIST, REG_FULL_RESOURCE_DESCRIPTOR and REG_RESOURCE_REQUIREMENTS_LIST types ?
    //      If not, should I specify an 'unknown' type in case such a value is readed into registry::value object ?

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

        /*! TODO: ... */
        create_sub_key =       0x00000004,

        /*! TODO: ... */
        enumerate_sub_keys =   0x00000008,

        /*! TODO: ... */
        execute =              0x00020019,

        /*! TODO: ... */
        notify =               0x00000010,

        /*! TODO: ... */
        query_value =          0x00000001,

        /*! TODO: ... */
        read =                 0x00020019,

        /*! TODO: ... */
        set_value =            0x00000002,

        /*! TODO: ... */
        write =                0x00020006,

        /*! TODO: ... */
        unknown =              0x00000000
    };

    /*! This type represents available options that control the behavior of the recursive_key_iterator. \n
    key_options satisfies the requirements of BitmaskType (which means the bitwise operators `operator&`, 
    `operator|`, `operator^`, `operator~`, `operator&=`, `operator|=`, and `operator^=` are defined for this type) */
    // TODO: integrate this to the recursive_key_iterator
    enum class key_options : uint16_t
    {
        /*! (Default) Permission denied is error. */
        none =                    0x0000,

        /*! Skip keys that would otherwise result in permission denied errors. */
        skip_permission_denied =  0x0001
    };

    /*! The Windows security model enables you to control access to registry keys. For more information see:
    https://msdn.microsoft.com/en-us/library/windows/desktop/ms724878 \n
    key_info_mask satisfies the requirements of BitmaskType (which means the bitwise operators `operator&`, 
    `operator|`, `operator^`, `operator~`, `operator&=`, `operator|=`, and `operator^=` are defined for this type) */
    enum class key_info_mask : uint16_t
    {
        /*! TODO: ... */
        none =                      0x0000,

        /*! TODO: ... */
        read_subkeys =              0x0001,

        /*! TODO: ... */
        read_values =               0x0002,

        /*! TODO: ... */
        read_max_subkey_size =      0x0004,

        /*! TODO: ... */
        read_max_value_name_size =  0x0008,

        /*! TODO: ... */
        read_max_value_data_size =  0x0010,

        /*! TODO: ... */
        read_last_write_time =      0x0020,

        /*! TODO: ... */
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

    namespace detail { inline constexpr std::error_code* throws() noexcept { return nullptr; } }

    // TODO: rename types.h to common.h ???
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

    constexpr key_options operator&(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator|(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator^(key_options lhs, key_options rhs) noexcept;

    constexpr key_options operator~(key_options lhs) noexcept;

    key_options& operator&=(key_options& lhs, key_options rhs) noexcept;

    key_options& operator|=(key_options& lhs, key_options rhs) noexcept;

    key_options& operator^=(key_options& lhs, key_options rhs) noexcept;

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

    inline constexpr key_options operator&(key_options lhs, key_options rhs) noexcept
    { return static_cast<key_options>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); }

    inline constexpr key_options operator|(key_options lhs, key_options rhs) noexcept
    { return static_cast<key_options>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); }

    inline constexpr key_options operator^(key_options lhs, key_options rhs) noexcept
    { return static_cast<key_options>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); }

    inline constexpr key_options operator~(key_options lhs) noexcept
    { return static_cast<key_options>(~static_cast<uint32_t>(lhs)); }

    inline key_options& operator&=(key_options& lhs, key_options rhs) noexcept { return lhs = lhs & rhs; }

    inline key_options& operator|=(key_options& lhs, key_options rhs) noexcept { return lhs = lhs | rhs; }

    inline key_options& operator^=(key_options& lhs, key_options rhs) noexcept { return lhs = lhs ^ rhs; }

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