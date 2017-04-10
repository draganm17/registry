#pragma once

#include <array>
#include <cstdint>
#include <time.h>
#include <Windows.h>

#include <boost/algorithm/string/predicate.hpp>

#include <registry/exception.h>
#include <registry/types.h>


#define VOID 

#define RETURN_RESULT(ec, result)          \
    {                                      \
        if (&ec != &throws()) ec.clear();  \
        return result;                     \
    }


namespace registry {
namespace details {

    template <class ...Args>
    inline void set_or_throw(std::error_code* ec_dst, const std::error_code& ec_src, const char* msg, Args&&... ex_args)
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

    inline string_view_type key_id_to_string(key_id id) noexcept
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
            default:                                   return string_view_type{};
        };
    }

    inline key_id key_id_from_string(string_view_type str) noexcept
    {
        using key_map_value_type = std::pair<string_view_type, key_id>;
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

        using boost::iequals;
        using boost::ilexicographical_compare;
        auto it = std::lower_bound(key_map.begin(), key_map.end(), str,
                                   [](auto&& lhs, auto&& rhs) { return ilexicographical_compare(lhs.first, rhs); });

        return (it != key_map.end() && iequals((*it).first, str)) ? (*it).second : key_id::unknown;
    }

}} // namespace registry::details