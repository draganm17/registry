// TODO: move to *.cpp file ???
#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <locale>
#include <string_view>
#include <system_error>
#include <time.h>
#include <Windows.h>

#include <registry/exception.h>
#include <registry/name.h>


#define VOID 

#define RETURN_RESULT(ec, result)          \
    {                                      \
        if (&ec != &throws()) ec.clear();  \
        return result;                     \
    }


namespace registry {
namespace details {

    struct is_iless
    {
        std::locale locale = std::locale();

        template< typename T1, typename T2>
        bool operator()(const T1& arg1, const T2& arg2) const 
        { return std::tolower(arg1, locale) < std::tolower(arg2, locale); }
    };

    struct is_iequal
    {
        std::locale locale = std::locale();

        template< typename T1, typename T2>
        bool operator()(const T1& arg1, const T2& arg2) const 
        { return std::tolower(arg1, locale) == std::tolower(arg2, locale); }
    };

    template <class T>
    inline void hash_combine(size_t& seed, const T& v)
    {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <class ...Args>
    inline void set_or_throw(std::error_code* ec_dst, 
                             const std::error_code& ec_src, const char* msg, Args&&... ex_args)
    {
        if (ec_dst) {
            *ec_dst = ec_src;
            return;
        }
        throw registry_error(ec_src, msg, std::forward<Args>(ex_args)...);
    }

    inline time_t file_time_to_time_t(const FILETIME& time) noexcept
    {
        const uint64_t t = (static_cast<uint64_t>(time.dwHighDateTime) << 32) | time.dwLowDateTime;
        return static_cast<time_t>((t - 116444736000000000ll) / 10000000);
    }

    inline std::basic_string_view<name::value_type> key_id_to_string(key_id id) noexcept
    {
        switch(id)
        {
            case key_id::classes_root :                return TEXT("HKEY_CLASSES_ROOT");
            case key_id::current_user :                return TEXT("HKEY_CURRENT_USER");
            case key_id::local_machine :               return TEXT("HKEY_LOCAL_MACHINE");
            case key_id::users :                       return TEXT("HKEY_USERS");
            case key_id::performance_data :            return TEXT("HKEY_PERFORMANCE_DATA");
            case key_id::performance_text :            return TEXT("HKEY_PERFORMANCE_TEXT");
            case key_id::performance_nlstext :         return TEXT("HKEY_PERFORMANCE_NLSTEXT");
            case key_id::current_config :              return TEXT("HKEY_CURRENT_CONFIG");
            case key_id::current_user_local_settings : return TEXT("HKEY_CURRENT_USER_LOCAL_SETTINGS");
            default:                                   return std::basic_string_view<name::value_type>();
        };
    }

    inline key_id key_id_from_string(std::basic_string_view<name::value_type> str) noexcept
    {
        using key_map_value_type = std::pair<std::basic_string_view<name::value_type>, key_id>;
        using key_map_type = std::array<key_map_value_type, 9>;

        // NOTE: keys are sorted in alphabetical order
        static const key_map_type key_map
        {
            key_map_value_type{ TEXT("HKEY_CLASSES_ROOT"),                key_id::classes_root                },
            key_map_value_type{ TEXT("HKEY_CURRENT_CONFIG"),              key_id::current_config              },
            key_map_value_type{ TEXT("HKEY_CURRENT_USER"),                key_id::current_user                },
            key_map_value_type{ TEXT("HKEY_CURRENT_USER_LOCAL_SETTINGS"), key_id::current_user_local_settings },
            key_map_value_type{ TEXT("HKEY_LOCAL_MACHINE"),               key_id::local_machine               },
            key_map_value_type{ TEXT("HKEY_PERFORMANCE_DATA"),            key_id::performance_data            },
            key_map_value_type{ TEXT("HKEY_PERFORMANCE_NLSTEXT"),         key_id::performance_nlstext         },
            key_map_value_type{ TEXT("HKEY_PERFORMANCE_TEXT"),            key_id::performance_text            },
            key_map_value_type{ TEXT("HKEY_USERS"),                       key_id::users                       }
        };

        static const auto cmp_less = [](auto&& lhs, auto&& rhs)
        {
            using std::begin; using std::end;
            return std::lexicographical_compare(begin(lhs.first), end(lhs.first), begin(rhs), end(rhs), is_iless());
        };

        static const auto cmp_equal = [](auto&& lhs, auto&& rhs)
        {
            using std::begin; using std::end;
            return std::equal(begin(lhs.first), end(lhs.first), begin(rhs), end(rhs), is_iequal());
        };

        auto it = std::lower_bound(key_map.begin(), key_map.end(), str, cmp_less);
        return (it != key_map.end() && cmp_equal(*it, str)) ? it->second : key_id::unknown;
    }

}} // namespace registry::details