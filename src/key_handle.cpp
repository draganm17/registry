#include <algorithm>
#include <array>
#include <cassert>
#include <locale>
#include <numeric>
#include <Windows.h>

#include <boost/algorithm/string.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/scope_exit.hpp>

#include <registry/key_handle.h>


namespace  {

using namespace registry;

class unique_hkey
{
    struct deleter
    {
        void operator()(void* hkey) const noexcept
        {
            switch ((ULONG_PTR)hkey) {
                case (ULONG_PTR)0x0:
                case (ULONG_PTR)HKEY_CLASSES_ROOT:
                case (ULONG_PTR)HKEY_CURRENT_USER:
                case (ULONG_PTR)HKEY_LOCAL_MACHINE:
                case (ULONG_PTR)HKEY_USERS:
                case (ULONG_PTR)HKEY_PERFORMANCE_DATA:
                case (ULONG_PTR)HKEY_PERFORMANCE_TEXT:
                case (ULONG_PTR)HKEY_PERFORMANCE_NLSTEXT:
                case (ULONG_PTR)HKEY_CURRENT_CONFIG:
                case (ULONG_PTR)HKEY_DYN_DATA:
                case (ULONG_PTR)HKEY_CURRENT_USER_LOCAL_SETTINGS: return;
            }
            ::RegCloseKey((HKEY)hkey);
        }
    };

    std::unique_ptr<void, deleter> m_handle;

public:
    constexpr unique_hkey(key_handle::native_handle_type hkey) noexcept
    : m_handle((void*)hkey, deleter{})
    { }

    operator key_handle::native_handle_type() const noexcept 
    { return (const key_handle::native_handle_type)m_handle.get(); }
};


const auto NtQueryKey_ = []() noexcept
{
    using F = DWORD(WINAPI*)(HANDLE, int, PVOID, ULONG, PULONG);
    return (F)GetProcAddress(LoadLibrary(TEXT("ntdll.dll")), "NtQueryKey");
}();

const auto RegDeleteKeyEx_ = []() noexcept
{
    using F = LONG(WINAPI*)(HKEY, LPCTSTR, REGSAM, DWORD);
    const auto osv = (DWORD)(LOBYTE(LOWORD(GetVersion())));
#if defined(_UNICODE)
    return osv > 5 ? (F)GetProcAddress(LoadLibrary(TEXT("Advapi32.dll")), "RegDeleteKeyExW") : nullptr;
#else
    return osv > 5 ? (F)GetProcAddress(LoadLibrary(TEXT("Advapi32.dll")), "RegDeleteKeyExA") : nullptr;
#endif
}();

std::uintmax_t remove_all_inside(const key& key, std::error_code& ec)
{
    /*
    ec.clear();
    //if (key.empty()) {
    //    return 0;
    //}

    std::uintmax_t keys_deleted = 0;
    std::vector<registry::key> rm_list;
    for (auto it = key_iterator(key, ec); !ec && it != key_iterator(); it.increment(ec)) {
        rm_list.push_back(*it);
        keys_deleted += remove_all_inside(*it, ec);
    }
    for (auto it = rm_list.begin(); !ec && it != rm_list.end(); ++it) {
        keys_deleted += remove(*it, ec) ? 1 : 0;
    }
    keys_deleted += 
        remove(registry::key(key).append(TEXT("Wow6432Node")).replace_view(view::view_64bit), ec) ? 1 : 0;

    return ec ? static_cast<std::uintmax_t>(-1) : keys_deleted;

    */

    // TODO: ...
    throw 0;
}

time_t file_time_to_time_t(const FILETIME time) noexcept
{
    const uint64_t t = (static_cast<uint64_t>(time.dwHighDateTime) << 32) | time.dwLowDateTime;
    return static_cast<time_t>((t - 116444736000000000ll) / 10000000);
}

std::wstring nt_name(key_handle::native_handle_type handle)
{
    static constexpr int   KEY_NAME_INFORMATION =    3;
    static constexpr DWORD STATUS_BUFFER_TOO_SMALL = 0xC0000023L;

    DWORD rc, size = 0;
    std::vector<uint8_t> buffer;
    HKEY hkey = reinterpret_cast<HKEY>(handle);

    do {
        rc = NtQueryKey_(hkey, KEY_NAME_INFORMATION, buffer.data(), size, &size);
        if (rc == STATUS_BUFFER_TOO_SMALL) buffer.resize(size += sizeof(wchar_t) * 2);
    } while (rc == STATUS_BUFFER_TOO_SMALL);
    return rc ? std::wstring() : std::wstring(reinterpret_cast<const wchar_t*>(buffer.data()) + 2);

    //TODO: this implementation assumes that no errors will occure in NtQueryKey
};

}  // anonymous namespace


namespace registry {

//------------------------------------------------------------------------------------//
//                                class key_handle                                    //
//------------------------------------------------------------------------------------//

struct key_handle::state
{
    unique_hkey    handle;
    access_rights  rights;
    registry::key  key;
};

key_handle::key_handle(const weak_key_handle& handle)
    : m_state(handle.m_state.lock())
{
    if (handle.expired()) throw bad_weak_key_handle();
}

key_handle::key_handle(key_id id, access_rights rights)
    : key_handle(static_cast<native_handle_type>(id), registry::key::from_key_id(id), rights)
{ }

key_handle::key_handle(native_handle_type handle, const registry::key& key, access_rights rights)
    : m_state(handle ? std::make_shared<state>(state{ handle, rights, key }) : nullptr)
{ }

key key_handle::key() const { return m_state ? m_state->key : registry::key(); }

access_rights key_handle::rights() const noexcept { return m_state ? m_state->rights : access_rights::unknown; }

key_handle::native_handle_type key_handle::native_handle() const noexcept
{
    return m_state ? static_cast<native_handle_type>(m_state->handle) : native_handle_type{};
}

bool key_handle::valid() const noexcept { return static_cast<bool>(m_state); }

bool key_handle::exists(string_view_type value_name) const
{
    std::error_code ec;
    decltype(auto) res = exists(value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), {}, value_name);
    return res;
}

bool key_handle::exists(string_view_type value_name, std::error_code& ec) const
{
    ec.clear();
    LSTATUS rc = RegQueryValueEx(reinterpret_cast<HKEY>(native_handle()), 
                                 value_name.data(), nullptr, nullptr, nullptr, nullptr);
    
    return (rc == ERROR_SUCCESS || 
            rc == ERROR_FILE_NOT_FOUND) ? (ec.clear(), rc != ERROR_FILE_NOT_FOUND)
                                        : (ec = std::error_code(rc, std::system_category()), false);
}

key_info key_handle::info(key_info_mask mask) const
{
    std::error_code ec;
    decltype(auto) res = info(mask, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key());
    return res;
}

key_info key_handle::info(key_info_mask mask, std::error_code& ec) const
{
    ec.clear();
    FILETIME time;
    key_info info{};

    const bool read_subkeys =             (mask & key_info_mask::read_subkeys)             != key_info_mask::none;
    const bool read_values =              (mask & key_info_mask::read_values)              != key_info_mask::none;
    const bool read_max_subkey_size =     (mask & key_info_mask::read_max_subkey_size)     != key_info_mask::none;
    const bool read_max_value_name_size = (mask & key_info_mask::read_max_value_name_size) != key_info_mask::none;
    const bool read_max_value_data_size = (mask & key_info_mask::read_max_value_data_size) != key_info_mask::none;
    const bool read_last_write_time =     (mask & key_info_mask::read_last_write_time)     != key_info_mask::none;

    LSTATUS rc = RegQueryInfoKey(
        reinterpret_cast<HKEY>(native_handle()), nullptr, nullptr, nullptr,
        read_subkeys             ? reinterpret_cast<DWORD*>(&info.subkeys)             : nullptr,
        read_max_subkey_size     ? reinterpret_cast<DWORD*>(&info.max_subkey_size)     : nullptr,
        nullptr,
        read_values              ? reinterpret_cast<DWORD*>(&info.values)              : nullptr,
        read_max_value_name_size ? reinterpret_cast<DWORD*>(&info.max_value_name_size) : nullptr,
        read_max_value_data_size ? reinterpret_cast<DWORD*>(&info.max_value_data_size) : nullptr,
        nullptr, 
        read_last_write_time     ? &time                                               : nullptr
    );
    ec = std::error_code(rc, std::system_category());

    if (!ec && read_last_write_time) {
        info.last_write_time = key_time_type::clock::from_time_t(file_time_to_time_t(time));
    }

    return !ec ? info : key_info{};
}

value key_handle::read_value(string_view_type value_name) const
{
    std::error_code ec;
    decltype(auto) res = read_value(value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), {}, value_name);
    return res;
}

value key_handle::read_value(string_view_type value_name, std::error_code& ec) const
{
    ec.clear();
    LSTATUS rc;
    details::value_state state;

    do {
        BYTE dummy;
        DWORD size = static_cast<DWORD>(state.m_data.size());
        rc = RegQueryValueEx(reinterpret_cast<HKEY>(native_handle()), value_name.data(), nullptr,
                             reinterpret_cast<DWORD*>(&state.m_type), size ? state.m_data.data() : &dummy, &size);
        state.m_data.resize(size);
    } while (rc == ERROR_MORE_DATA);
        
    return (rc == ERROR_SUCCESS) ? reinterpret_cast<value&&>(state)
                                 : (ec = std::error_code(rc, std::system_category()), value());
}

std::pair<key_handle, bool> key_handle::create_key(const registry::key& subkey, access_rights rights) const
{
    std::error_code ec;
    decltype(auto) res = create_key(subkey, rights, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), subkey);
    return res;
}

std::pair<key_handle, bool> key_handle::create_key(const registry::key& subkey, access_rights rights, std::error_code& ec) const
{
    ec.clear();
    DWORD disp;
    key_handle::native_handle_type hkey;
    const DWORD sam_desired = static_cast<DWORD>(rights) | static_cast<DWORD>(subkey.view());
    LSTATUS rc = RegCreateKeyEx(reinterpret_cast<HKEY>(native_handle()), subkey.name().data(), 0, nullptr,
                                REG_OPTION_NON_VOLATILE, sam_desired, nullptr, reinterpret_cast<HKEY*>(&hkey), &disp);
    
    if (rc == ERROR_SUCCESS) {
        // NOTE: the new key will have the same view as the subkey.
        auto new_key = registry::key(key().name(), subkey.view()).append(subkey.name());
        return std::make_pair(key_handle(hkey, std::move(new_key), rights), disp == REG_CREATED_NEW_KEY);
    }
    return (ec = std::error_code(rc, std::system_category()), std::make_pair(key_handle(), false));
}

void key_handle::write_value(string_view_type value_name, const value& value) const
{
    std::error_code ec;
    write_value(value_name, value, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), {}, value_name);
}

void key_handle::write_value(string_view_type value_name, const value& value, std::error_code& ec) const
{
    ec.clear();
    LSTATUS rc = RegSetValueEx(reinterpret_cast<HKEY>(native_handle()), 
                               value_name.data(), 0, static_cast<DWORD>(value.type()), 
                               value.data().data(), static_cast<DWORD>(value.data().size()));
    
    if (rc != ERROR_SUCCESS) ec = std::error_code(rc, std::system_category());
}

bool key_handle::remove(const registry::key& subkey) const
{
    std::error_code ec;
    decltype(auto) res = remove(subkey, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), subkey);
    return res;
}

bool key_handle::remove(const registry::key& subkey, std::error_code& ec) const
{
    ec.clear();
    LSTATUS rc;
    if (!RegDeleteKeyEx_) {
        rc = RegDeleteKey(reinterpret_cast<HKEY>(native_handle()), subkey.name().data());
    } else {
        rc = RegDeleteKeyEx_(reinterpret_cast<HKEY>(native_handle()),
                             subkey.name().data(), static_cast<DWORD>(subkey.view()), 0);
    }

    return (rc == ERROR_SUCCESS ||
            rc == ERROR_FILE_NOT_FOUND) ? (ec.clear(), rc != ERROR_FILE_NOT_FOUND)
                                        : (ec = std::error_code(rc, std::system_category()), false);
}

bool key_handle::remove(string_view_type value_name) const
{
    std::error_code ec;
    decltype(auto) res = remove(value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), {}, value_name);
    return res;
}

bool key_handle::remove(string_view_type value_name, std::error_code& ec) const
{
    ec.clear();
    LSTATUS rc = RegDeleteValue(reinterpret_cast<HKEY>(native_handle()), value_name.data());

    return (rc == ERROR_SUCCESS || 
            rc == ERROR_FILE_NOT_FOUND) ? (ec.clear(), rc != ERROR_FILE_NOT_FOUND)
                                        : (ec = std::error_code(rc, std::system_category()), false);
}

std::uintmax_t key_handle::remove_all(const registry::key& subkey) const
{
    std::error_code ec;
    decltype(auto) res = remove_all(subkey, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), subkey);
    return res;
}

std::uintmax_t key_handle::remove_all(const registry::key& subkey, std::error_code& ec) const
{
    // TODO: ...
    return 0;
}

bool key_handle::equivalent(const registry::key& key) const
{
    std::error_code ec;
    decltype(auto) res = equivalent(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, this->key(), key);
    return res;
}

bool key_handle::equivalent(const registry::key& key, std::error_code& ec) const
{
    ec.clear();
    auto handle = open(key, access_rights::query_value, ec);
    return !ec ? equivalent(handle) : false;
}

bool key_handle::equivalent(const key_handle& handle) const
{
    std::error_code ec;
    decltype(auto) res = equivalent(handle, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), handle.key());
    return res;
}

bool key_handle::equivalent(const key_handle& handle, std::error_code& ec) const
{
    ec.clear();
    return nt_name(native_handle()) == nt_name(handle.native_handle());
}

void key_handle::swap(key_handle& other) noexcept { m_state.swap(other.m_state); }


//------------------------------------------------------------------------------------//
//                            class weak_key_handle                                   //
//------------------------------------------------------------------------------------//

weak_key_handle::weak_key_handle(const key_handle& handle) noexcept
    : m_state(handle.m_state)
{ }

weak_key_handle& weak_key_handle::operator=(const key_handle& other) noexcept 
{ 
    m_state = other.m_state;
    return *this;
}

bool weak_key_handle::expired() const noexcept { return m_state.expired(); }

key_handle weak_key_handle::lock() const noexcept
{
    key_handle handle;
    handle.m_state = m_state.lock();
    return handle;
}

void weak_key_handle::swap(weak_key_handle& other) noexcept { m_state.swap(other.m_state); }

}  // namespace registry