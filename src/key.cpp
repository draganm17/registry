#include <algorithm>
#include <cassert>
#include <Windows.h>

#include <registry/config.h>
#include <registry/exception.h>
#include <registry/details/utils.impl.h>
#include <registry/key.h>
#include <registry/key_iterator.h>


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
    return osv >= 6 ? (F)GetProcAddress(LoadLibrary(TEXT("Advapi32.dll")), "RegDeleteKeyExW") : nullptr;
#else
    return osv >= 6 ? (F)GetProcAddress(LoadLibrary(TEXT("Advapi32.dll")), "RegDeleteKeyExA") : nullptr;
#endif
}();
#endif

inline constexpr bool& dont_care() noexcept { return (bool&)(*((bool*)nullptr)); }

void close_handle(key::native_handle_type handle, std::error_code& ec) noexcept
{
    ec.clear();
    switch ((ULONG_PTR)handle) {
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

    const LSTATUS rc = RegCloseKey((HKEY)handle);
    if (rc != ERROR_SUCCESS) ec = std::error_code(rc, std::system_category());
}

uint32_t remove_all_inside(const key& key, const key_path& path, std::error_code& ec)
{
    // TODO: ...
    return 0;

    /*
    ec.clear();
    auto subkey = key.open_key(path, access_rights::read, ec);
    
    if (ec) {
        return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), 0)
                                                    : static_cast<uint32_t>(-1);
    }

    uint32_t keys_deleted = 0;
    std::vector<key_path> rm_list;
    for (auto it = key_iterator(subkey, ec); !ec && it != key_iterator(); it.increment(ec))
    {
        if (ec) break;
        rm_list.push_back(it->path().leaf_key());
        keys_deleted += remove_all_inside(subkey, rm_list.back(), ec);
    }

    for (auto it = rm_list.begin(); !ec && it != rm_list.end(); ++it) {
        keys_deleted += subkey.remove_key(*it, ec);
    }

    return !ec ? keys_deleted : static_cast<uint32_t>(-1);
    */
}

std::wstring nt_key_name(key::native_handle_type handle)
{
    static constexpr int   KEY_NAME_INFORMATION =    3;
    static constexpr DWORD STATUS_BUFFER_TOO_SMALL = 0xC0000023L;

    DWORD rc, size = 0;
    std::vector<uint8_t> buffer;
    HKEY hkey = reinterpret_cast<HKEY>(handle);

    do {
        rc = NtQueryKey_(hkey, KEY_NAME_INFORMATION, buffer.data(), size, &size);
        if (rc == STATUS_BUFFER_TOO_SMALL) buffer.resize(size += sizeof(ULONG));
    } while (rc == STATUS_BUFFER_TOO_SMALL);

    return rc ? std::wstring() 
              : std::wstring(reinterpret_cast<const wchar_t*>(buffer.data() + sizeof(ULONG)));

    //TODO: this implementation assumes that no errors will occure in NtQueryKey
};

}  // anonymous namespace


namespace registry {

//------------------------------------------------------------------------------------//
//                                   class key                                        //
//------------------------------------------------------------------------------------//

void key::close_handle_t::operator()(void* hkey) const noexcept
{
    std::error_code ec;
    close_handle(reinterpret_cast<native_handle_type>(hkey), ec);
}

key::key(key_id id)
    : m_rights(access_rights::unknown)
    , m_handle(reinterpret_cast<void*>(id))
{ }

key::key(open_only_tag, const key_path& path, access_rights rights, std::error_code& ec)
    : m_rights(rights)
{
    LRESULT rc = ERROR_FILE_NOT_FOUND;

    if (path.is_absolute())
    {
        HKEY hkey;
        rc = RegOpenKeyEx(reinterpret_cast<HKEY>(path.root_key_id()),
                          path.relative_path().key_name().data(), 0,
                          static_cast<DWORD>(rights) | static_cast<DWORD>(path.key_view()), &hkey);

        if (rc == ERROR_SUCCESS) {
            m_handle = handle_t(reinterpret_cast<void*>(hkey));
            RETURN_RESULT(ec, VOID);
        }
    }

    swap(key()); // close this key
    details::set_or_throw(&ec, std::error_code(rc, std::system_category()), __FUNCTION__, path);
}

key::key(open_or_create_tag, const key_path& path, access_rights rights, std::error_code& ec)
    : key(open_or_create_tag{}, path, rights, dont_care(), ec)
{ }

key::key(open_or_create_tag, const key_path& path, access_rights rights, bool& was_created, std::error_code& ec)
    : m_rights(rights)
{
    std::error_code ec2;
    key_path lpath = path, rpath(path.key_view());
    if (&was_created != &dont_care()) was_created = false;
    key lkey(open_only_tag{}, lpath, access_rights::create_sub_key, ec2);

    while (ec2.value() == ERROR_FILE_NOT_FOUND && lpath.has_parent_path()) {
        rpath = lpath.leaf_path().append(rpath);
        lkey = key(open_only_tag{}, lpath.remove_leaf_path(), access_rights::create_sub_key, ec2);
    }

    std::pair<key, bool> create_key_result;
    if (!ec2 && (create_key_result = lkey.create_key(rpath, rights, ec2), !ec2))
    {
        m_handle.swap(create_key_result.first.m_handle);
        if (&was_created != &dont_care()) was_created = create_key_result.second;
        RETURN_RESULT(ec, VOID);
    }

    swap(key()); // close this key
    details::set_or_throw(&ec, ec2, __FUNCTION__, path);
}

access_rights key::rights() const noexcept { return is_open() ? m_rights : access_rights::unknown; }

key::native_handle_type key::native_handle() const noexcept
{ return reinterpret_cast<native_handle_type>(m_handle.get()); }

bool key::is_open() const noexcept { return static_cast<bool>(m_handle); }

value_iterator key::get_value_iterator(std::error_code& ec) const
{
    std::error_code ec2;
    value_iterator it(*this, ec2);

    if (!ec2) RETURN_RESULT(ec, it);
    return details::set_or_throw(&ec, ec2, __FUNCTION__), value_iterator();
}

std::pair<key, bool> key::create_key(const key_path& path, access_rights rights, std::error_code& ec)
{
    key key;
    HKEY hkey;
    DWORD disp;
    key.m_rights = rights;
    const DWORD sam_desired = static_cast<DWORD>(rights) | static_cast<DWORD>(path.key_view());
    const LSTATUS rc = RegCreateKeyEx(reinterpret_cast<HKEY>(native_handle()), path.key_name().data(),
                                      0, nullptr, REG_OPTION_NON_VOLATILE, sam_desired, nullptr, &hkey, &disp);
    
    if (rc == ERROR_SUCCESS) {
        key.m_handle = handle_t(reinterpret_cast<void*>(hkey));
        RETURN_RESULT(ec, std::make_pair(std::move(key), disp == REG_CREATED_NEW_KEY));
    }

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), std::make_pair(registry::key(), false);
}

bool key::equivalent(const key_path& path, std::error_code& ec) const
{
    std::error_code ec2;
    const auto key = open_key(path, access_rights::query_value, ec2);

    bool result;
    if (!ec2 && (result = equivalent(key, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

bool key::equivalent(const key& key, std::error_code& ec) const
{ RETURN_RESULT(ec, nt_key_name(native_handle()) == nt_key_name(key.native_handle())); }

key_info key::info(key_info_mask mask, std::error_code& ec) const
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
    return details::set_or_throw(&ec, ec2, __FUNCTION__), invalid_info;
}

bool key::key_exists(const key_path& path, std::error_code& ec) const
{
    std::error_code ec2;
    open_key(path, access_rights::read, ec2);

    if (!ec2) RETURN_RESULT(ec, true);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

key key::open_key(const key_path& path, access_rights rights, std::error_code& ec) const
{
    key key;
    HKEY hkey;
    key.m_rights = rights;
    const LRESULT rc = RegOpenKeyEx(reinterpret_cast<HKEY>(native_handle()), path.key_name().data(), 0,
                                    static_cast<DWORD>(rights) | static_cast<DWORD>(path.key_view()), &hkey);

    if (rc == ERROR_SUCCESS) {
        key.m_handle = handle_t(reinterpret_cast<void*>(hkey));
        RETURN_RESULT(ec, key);
    }

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), registry::key();
}

value key::read_value(string_view_type value_name, std::error_code& ec) const
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
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key_path(), key_path(), value_name), value();
}

bool key::remove_key(const key_path& path, std::error_code& ec)
{
    LSTATUS rc;
#if REGISTRY_USE_WINAPI_VERSION < REGISTRY_WINAPI_VERSION_VISTA
    if (!RegDeleteKeyEx_) {
        rc = RegDeleteKey(reinterpret_cast<HKEY>(native_handle()), subkey.key_name().data());
    } else {
        rc = RegDeleteKeyEx_(reinterpret_cast<HKEY>(native_handle()),
                             subkey.key_name().data(), static_cast<DWORD>(subkey.key_view()), 0);
    }
#else
    rc = RegDeleteKeyEx(reinterpret_cast<HKEY>(native_handle()),
                        path.key_name().data(), static_cast<DWORD>(path.key_view()), 0);
#endif

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, true);
    if (rc == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

uint32_t key::remove_keys(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    uint32_t keys_deleted = 0;
    if ((keys_deleted += remove_all_inside(*this, path, ec2), !ec2) &&
        (keys_deleted += static_cast<uint32_t>(remove_key(path, ec2)), !ec2))
    {
        RETURN_RESULT(ec, keys_deleted);
    }
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), static_cast<uint32_t>(-1);
}

bool key::remove_value(string_view_type value_name, std::error_code& ec)
{
    const LSTATUS rc = RegDeleteValue(reinterpret_cast<HKEY>(native_handle()), value_name.data());

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, true);
    if (rc == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key_path(), key_path(), value_name), false;
}

bool key::value_exists(string_view_type value_name, std::error_code& ec) const
{
    const LSTATUS rc = RegQueryValueEx(reinterpret_cast<HKEY>(native_handle()), 
                                       value_name.data(), nullptr, nullptr, nullptr, nullptr);

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, true);
    if (rc == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key_path(), key_path(), value_name), false;
}

void key::write_value(string_view_type value_name, const value& value, std::error_code& ec)
{
    const LSTATUS rc = RegSetValueEx(reinterpret_cast<HKEY>(native_handle()), 
                                     value_name.data(), 0, static_cast<DWORD>(value.type()), 
                                     value.data().data(), static_cast<DWORD>(value.data().size()));
    
    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, VOID);
    const std::error_code ec2(rc, std::system_category());
    details::set_or_throw(&ec, ec2, __FUNCTION__, key_path(), key_path(), value_name);
}

void key::close(std::error_code& ec)
{
    std::error_code ec2;
    key tmp(std::move(*this));
    if (close_handle(tmp.m_handle.release(), ec2), !ec2) RETURN_RESULT(ec, VOID);

    details::set_or_throw(&ec, ec2, __FUNCTION__);
}

void key::swap(key& other) noexcept
{ 
    using std::swap;
    swap(m_rights, other.m_rights);
    swap(m_handle, other.m_handle);
}

}  // namespace registry