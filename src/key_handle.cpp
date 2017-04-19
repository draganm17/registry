#include <algorithm>
#include <cassert>
#include <Windows.h>

#include <registry/config.h>
#include <registry/exception.h>
#include <registry/details/utils.impl.h>
#include <registry/key_handle.h>
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
    return osv > 5 ? (F)GetProcAddress(LoadLibrary(TEXT("Advapi32.dll")), "RegDeleteKeyExW") : nullptr;
#else
    return osv > 5 ? (F)GetProcAddress(LoadLibrary(TEXT("Advapi32.dll")), "RegDeleteKeyExA") : nullptr;
#endif
}();
#endif

void close_handle(key_handle::native_handle_type handle, std::error_code& ec) noexcept
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

uint32_t remove_all_inside(const key_handle& handle, const key_path& path, std::error_code& ec)
{
    ec.clear();
    const auto subkey_handle = handle.open(path, access_rights::query_value, ec);
    
    if (ec) {
        return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), 0)
                                                    : static_cast<uint32_t>(-1);
    }

    uint32_t keys_deleted = 0;
    std::vector<key_path> rm_list;
    for (auto it = key_iterator(subkey_handle.path(), ec); !ec && it != key_iterator(); it.increment(ec))
    {
        if (ec) break;
        rm_list.push_back(it->path().leaf_key());
        keys_deleted += remove_all_inside(subkey_handle, rm_list.back(), ec);
    }

    for (auto it = rm_list.begin(); !ec && it != rm_list.end(); ++it) {
        keys_deleted += subkey_handle.remove(*it, ec);
    }

    return !ec ? keys_deleted : static_cast<uint32_t>(-1);
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

void key_handle::close_handle_t::operator()(void* hkey) const noexcept
{
    std::error_code ec;
    close_handle(reinterpret_cast<native_handle_type>(hkey), ec);
}

key_handle::key_handle(key_id id)
    : m_path(key_path::from_key_id(id))
    , m_rights(access_rights::unknown)
    , m_handle(reinterpret_cast<void*>(id), close_handle_t{})
{ }

key_handle::key_handle(const key_path& path, access_rights rights, std::error_code& ec)
    : m_path(path)
    , m_rights(rights)
{
    LRESULT rc = ERROR_FILE_NOT_FOUND;

    if (path.is_absolute())
    {
        HKEY hkey;
        const LRESULT rc = RegOpenKeyEx(reinterpret_cast<HKEY>(path.root_key_id()),
                                        path.has_parent_key() ? (++path.begin())->data() : TEXT(""), 0,
                                        static_cast<DWORD>(rights) | static_cast<DWORD>(path.view()), &hkey);

        if (rc == ERROR_SUCCESS) {
            m_handle = std::unique_ptr<void, close_handle_t>(reinterpret_cast<void*>(hkey), close_handle_t{});
            RETURN_RESULT(ec, VOID);
        }
    }

    swap(key_handle()); // close this handle
    details::set_or_throw(&ec, std::error_code(rc, std::system_category()), __FUNCTION__, path);
}

key_path key_handle::path() const { return is_open() ? m_path : key_path(); }

access_rights key_handle::rights() const noexcept { return is_open() ? m_rights : access_rights::unknown; }

key_handle::native_handle_type key_handle::native_handle() const noexcept
{ return reinterpret_cast<native_handle_type>(m_handle.get()); }

bool key_handle::is_open() const noexcept { return static_cast<bool>(m_handle); }

std::pair<key_handle, bool> key_handle::create_key(const key_path& path, 
                                                   access_rights rights, std::error_code& ec) const
{
    HKEY hkey;
    DWORD disp;
    key_handle handle;
    handle.m_rights = rights;
    handle.m_path = this->path().append(path);
    const DWORD sam_desired = static_cast<DWORD>(rights) | static_cast<DWORD>(path.view());
    const LSTATUS rc = RegCreateKeyEx(reinterpret_cast<HKEY>(native_handle()), path.name().data(),
                                      0, nullptr, REG_OPTION_NON_VOLATILE, sam_desired, nullptr, &hkey, &disp);
    
    if (rc == ERROR_SUCCESS) {
        handle.m_handle = std::unique_ptr<void, close_handle_t>(reinterpret_cast<void*>(hkey), close_handle_t{});
        RETURN_RESULT(ec, std::make_pair(std::move(handle), disp == REG_CREATED_NEW_KEY));
    }

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, this->path(), path), std::make_pair(key_handle(), false);
}

bool key_handle::equivalent(const key_path& path, std::error_code& ec) const
{
    std::error_code ec2;
    bool result = false;
    const auto handle = open(path, access_rights::query_value, ec2);

    if (!ec2 && (result = equivalent(handle, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, this->path(), path), result;
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
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path(), key_path(), value_name), false;
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
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path()), invalid_info;
}

key_handle key_handle::open(const key_path& path, access_rights rights, std::error_code& ec) const
{
    HKEY hkey;
    key_handle handle;
    handle.m_rights = rights;
    handle.m_path = this->path().append(path);
    const LRESULT rc = RegOpenKeyEx(reinterpret_cast<HKEY>(native_handle()), path.name().data(), 0,
                                    static_cast<DWORD>(rights) | static_cast<DWORD>(path.view()), &hkey);

    if (rc == ERROR_SUCCESS) {
        handle.m_handle = std::unique_ptr<void, close_handle_t>(reinterpret_cast<void*>(hkey), close_handle_t{});
        RETURN_RESULT(ec, handle);
    }

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, this->path(), path), key_handle();
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
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path(), key_path(), value_name), value();
}

bool key_handle::remove(const key_path& path, std::error_code& ec) const
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
                        path.name().data(), static_cast<DWORD>(path.view()), 0);
#endif

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, true);
    if (rc == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, this->path(), path), false;
}

bool key_handle::remove(string_view_type value_name, std::error_code& ec) const
{
    const LSTATUS rc = RegDeleteValue(reinterpret_cast<HKEY>(native_handle()), value_name.data());

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, true);
    if (rc == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path(), key_path(), value_name), false;
}

uint32_t key_handle::remove_all(const key_path& path, std::error_code& ec) const
{
    std::error_code ec2;
    uint32_t keys_deleted = 0;
    if ((keys_deleted += remove_all_inside(*this, path, ec2), !ec2) &&
        (keys_deleted += static_cast<uint32_t>(remove(path, ec2)), !ec2))
    {
        RETURN_RESULT(ec, keys_deleted);
    }
    return details::set_or_throw(&ec, ec2, __FUNCTION__, this->path(), path), static_cast<uint32_t>(-1);
}

void key_handle::write_value(string_view_type value_name, const value& value, std::error_code& ec) const
{
    const LSTATUS rc = RegSetValueEx(reinterpret_cast<HKEY>(native_handle()), 
                                     value_name.data(), 0, static_cast<DWORD>(value.type()), 
                                     value.data().data(), static_cast<DWORD>(value.data().size()));
    
    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, VOID);
    const std::error_code ec2(rc, std::system_category());
    details::set_or_throw(&ec, ec2, __FUNCTION__, path(), key_path(), value_name);
}

void key_handle::close(std::error_code& ec)
{
    std::error_code ec2;
    key_handle tmp(std::move(*this));
    if (close_handle(tmp.m_handle.release(), ec2), !ec2) RETURN_RESULT(ec, VOID);

    details::set_or_throw(&ec, ec2, __FUNCTION__, tmp.m_path);
}

void key_handle::swap(key_handle& other) noexcept
{ 
    using std::swap;
    swap(m_path, other.m_path);
    swap(m_rights, other.m_rights);
    swap(m_handle, other.m_handle);
}


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

size_t hash_value(const key_handle& handle) noexcept
{ return std::hash<key_handle::native_handle_type>()(handle.native_handle()); }

}  // namespace registry