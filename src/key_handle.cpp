#include <algorithm>
#include <cassert>
#include <Windows.h>

#include <registry/config.h>
#include <registry/exception.h>
#include <registry/details/utils.impl.h>
#include <registry/key_handle.h>
//#include <registry/key_iterator.h> // TODO: ...


namespace  {

using namespace registry;

const auto NtQueryKey_ = []() noexcept
{
    using F = DWORD(WINAPI*)(HANDLE, int, PVOID, ULONG, PULONG);
    return (F)GetProcAddress(LoadLibrary(TEXT("ntdll.dll")), "NtQueryKey");
}();

#if REGISTRY_USE_WINAPI_VERSION < REGISTRY_WINAPI_VERSION_VISTA
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
#endif

uint32_t remove_all_inside(const key_handle& handle, const registry::key& subkey, std::error_code& ec)
{
    /*
    ec.clear();
    constexpr auto perms = access_rights::query_value |
                           access_rights::enumerate_sub_keys;
    const auto subkey_handle = handle.open(subkey, perms, ec);
    
    if (ec) {
        return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), 0)
                                                    : static_cast<uint32_t>(-1);
    }

    uint32_t keys_deleted = 0;
    std::vector<registry::key> rm_list;
    for (auto it = key_iterator(subkey_handle, ec); !ec && it != key_iterator(); it.increment(ec))
    {
        if (ec) break;
        rm_list.push_back(it->key().leaf_key());
        keys_deleted += remove_all_inside(subkey_handle, rm_list.back(), ec);
    }

    for (auto it = rm_list.begin(); !ec && it != rm_list.end(); ++it) {
        keys_deleted += subkey_handle.remove(*it, ec);
    }

    return !ec ? keys_deleted : static_cast<uint32_t>(-1);
    */

    // TODO: ...
    return 0;
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

void key_handle::close_handle::operator()(void* hkey) const noexcept
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

key_handle::key_handle(key_id id)
    : m_key(key::from_key_id(id))
    , m_rights(access_rights::unknown)
    , m_handle(reinterpret_cast<void*>(id), close_handle{})
{ }

key_handle::key_handle(const registry::key& key, access_rights rights, std::error_code& ec)
{
    // TODO: ...

    /*
    std::error_code ec2;
    if (!key.is_absolute()) {
        ec2 = std::error_code(ERROR_FILE_NOT_FOUND, std::system_category());
        return details::set_or_throw(&ec, ec2, __FUNCTION__, key), key_handle();
    }

    const key_handle root = key.root_key_id();
    const registry::key subkey(key.has_parent_key() ? (++key.begin())->data() : TEXT(""), key.view());

    auto handle = root.open(subkey, rights, ec2);

    if (!ec2) RETURN_RESULT(ec, handle);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key), key_handle();
    */
}

key key_handle::key() const { return m_key; }

access_rights key_handle::rights() const noexcept { return m_rights; }

key_handle::native_handle_type key_handle::native_handle() const noexcept
{ return reinterpret_cast<native_handle_type>(m_handle.get()); }

bool key_handle::is_open() const noexcept { return static_cast<bool>(m_handle); }

std::pair<key_handle, bool> key_handle::create_key(const registry::key& subkey, 
                                                   access_rights rights, std::error_code& ec) const
{
    /*
    DWORD disp;
    key_handle::native_handle_type hkey;
    auto new_key = key().append(subkey);
    const DWORD sam_desired = static_cast<DWORD>(rights) | static_cast<DWORD>(subkey.view());
    LSTATUS rc = RegCreateKeyEx(reinterpret_cast<HKEY>(native_handle()), subkey.name().data(), 0, nullptr,
                                REG_OPTION_NON_VOLATILE, sam_desired, nullptr, reinterpret_cast<HKEY*>(&hkey), &disp);
    
    if (rc == ERROR_SUCCESS) {
        RETURN_RESULT(ec, std::make_pair(key_handle(hkey, std::move(new_key), rights), disp == REG_CREATED_NEW_KEY));
    }
    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key(), subkey), std::make_pair(key_handle(), false);
    */

    // TODO: ...
    return std::pair<key_handle, bool>{};
}

bool key_handle::equivalent(const registry::key& key, std::error_code& ec) const
{
    std::error_code ec2;
    bool result = false;
    const auto handle = open(key, access_rights::query_value, ec2);

    if (!ec2 && (result = equivalent(handle, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, this->key(), key), result;
}

bool key_handle::equivalent(const key_handle& handle, std::error_code& ec) const
{
    RETURN_RESULT(ec, nt_name(native_handle()) == nt_name(handle.native_handle()));
}

bool key_handle::exists(string_view_type value_name, std::error_code& ec) const
{
    const LSTATUS rc = RegQueryValueEx(reinterpret_cast<HKEY>(native_handle()), 
                                       value_name.data(), nullptr, nullptr, nullptr, nullptr);

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, true);
    if (rc == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key(), registry::key(), value_name), false;
}

key_info key_handle::info(key_info_mask mask, std::error_code& ec) const
{
    constexpr key_info invalid_info{ uint32_t(-1), uint32_t(-1), uint32_t(-1), 
                                     uint32_t(-1), uint32_t(-1), key_time_type::min() };

    const bool read_subkeys =             (mask & key_info_mask::read_subkeys)             != key_info_mask::none;
    const bool read_values =              (mask & key_info_mask::read_values)              != key_info_mask::none;
    const bool read_max_subkey_size =     (mask & key_info_mask::read_max_subkey_size)     != key_info_mask::none;
    const bool read_max_value_name_size = (mask & key_info_mask::read_max_value_name_size) != key_info_mask::none;
    const bool read_max_value_data_size = (mask & key_info_mask::read_max_value_data_size) != key_info_mask::none;
    const bool read_last_write_time =     (mask & key_info_mask::read_last_write_time)     != key_info_mask::none;

    FILETIME time;
    key_info info = invalid_info;
    const LSTATUS rc = RegQueryInfoKey(
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

    if (rc == ERROR_SUCCESS) {
        if (read_last_write_time) 
            info.last_write_time = key_time_type::clock::from_time_t(details::file_time_to_time_t(time));
        RETURN_RESULT(ec, info);
    }

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key()), invalid_info;
}

key_handle key_handle::open(const registry::key& subkey, access_rights rights, std::error_code& ec) const
{
    /*
    LRESULT rc;
    key_handle::native_handle_type hkey;
    registry::key opened_key = key().append(subkey);
    rc = RegOpenKeyEx(reinterpret_cast<HKEY>(native_handle()), subkey.name().data(), 0,
                      static_cast<DWORD>(rights) | static_cast<DWORD>(subkey.view()), reinterpret_cast<HKEY*>(&hkey));

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, key_handle(hkey, std::move(opened_key), rights));

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key(), subkey), key_handle();
    */

    // TODO: ...
    return key_handle();
}

value key_handle::read_value(string_view_type value_name, std::error_code& ec) const
{
    LSTATUS rc;
    details::value_state state;

    do {
        BYTE dummy;
        DWORD size = static_cast<DWORD>(state.m_data.size());
        rc = RegQueryValueEx(reinterpret_cast<HKEY>(native_handle()), value_name.data(), nullptr,
                             reinterpret_cast<DWORD*>(&state.m_type), size ? state.m_data.data() : &dummy, &size);
        state.m_data.resize(size);
    } while (rc == ERROR_MORE_DATA);
    
    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, reinterpret_cast<value&&>(state));

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key(), registry::key(), value_name), value();
}

bool key_handle::remove(const registry::key& subkey, std::error_code& ec) const
{
    LSTATUS rc;
#if REGISTRY_USE_WINAPI_VERSION < REGISTRY_WINAPI_VERSION_VISTA
    if (!RegDeleteKeyEx_) {
        rc = RegDeleteKey(reinterpret_cast<HKEY>(native_handle()), subkey.name().data());
    } else {
        rc = RegDeleteKeyEx_(reinterpret_cast<HKEY>(native_handle()),
                             subkey.name().data(), static_cast<DWORD>(subkey.view()), 0);
    }
#else
    rc = RegDeleteKeyEx(reinterpret_cast<HKEY>(native_handle()),
                        subkey.name().data(), static_cast<DWORD>(subkey.view()), 0);
#endif

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, true);
    if (rc == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key(), subkey), false;
}

bool key_handle::remove(string_view_type value_name, std::error_code& ec) const
{
    const LSTATUS rc = RegDeleteValue(reinterpret_cast<HKEY>(native_handle()), value_name.data());

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, true);
    if (rc == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key(), registry::key(), value_name), false;
}

uint32_t key_handle::remove_all(const registry::key& subkey, std::error_code& ec) const
{
    std::error_code ec2;
    uint32_t keys_deleted = 0;
    if ((keys_deleted += remove_all_inside(*this, subkey, ec2), !ec2) &&
        (keys_deleted += static_cast<uint32_t>(remove(subkey, ec2)), !ec2))
    {
        RETURN_RESULT(ec, keys_deleted);
    }
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key(), subkey), static_cast<uint32_t>(-1);
}

void key_handle::write_value(string_view_type value_name, const value& value, std::error_code& ec) const
{
    const LSTATUS rc = RegSetValueEx(reinterpret_cast<HKEY>(native_handle()), 
                                     value_name.data(), 0, static_cast<DWORD>(value.type()), 
                                     value.data().data(), static_cast<DWORD>(value.data().size()));
    
    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, VOID);
    const std::error_code ec2(rc, std::system_category());
    details::set_or_throw(&ec, ec2, __FUNCTION__, key(), registry::key(), value_name);
}

void key_handle::close(std::error_code& ec)
{
    // TODO: ...
}

void key_handle::swap(key_handle& other) noexcept
{ 
    using std::swap;
    swap(m_key, other.m_key);
    swap(m_rights, other.m_rights);
    swap(m_handle, other.m_handle);
}

}  // namespace registry