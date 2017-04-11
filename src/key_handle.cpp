#include <algorithm>
#include <cassert>
#include <Windows.h>

#include <boost/algorithm/string.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/scope_exit.hpp>

#include <registry/exception.h>
#include <registry/details/utils.impl.h>
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
    constexpr unique_hkey(key_id id) noexcept
    : m_handle((void*)id, deleter{})
    { }

    constexpr unique_hkey(key_handle::native_handle_type hkey) noexcept
    : m_handle((void*)hkey, deleter{})
    { }

    operator key_id() const noexcept
    { return (key_id)((uintptr_t)m_handle.get()); }

    operator key_handle::native_handle_type() const noexcept 
    { return reinterpret_cast<key_handle::native_handle_type>(m_handle.get()); }
};

struct key_handle_state
{
    unique_hkey    handle = key_id::unknown;
    access_rights  rights = access_rights::unknown;
    registry::key  key =    key::from_key_id(handle);
};

class key_handle_pool
{
private:
    struct pool_t
    {
        key_handle_state classes_root                { key_id::classes_root };
        key_handle_state current_user                { key_id::current_user };
        key_handle_state local_machine               { key_id::local_machine };
        key_handle_state users                       { key_id::users };
        key_handle_state performance_data            { key_id::performance_data };
        key_handle_state performance_text            { key_id::performance_text };
        key_handle_state performance_nlstext         { key_id::performance_nlstext };
        key_handle_state current_config              { key_id::current_config };
        key_handle_state current_user_local_settings { key_id::current_user_local_settings };
    };

private:
    key_handle_pool() = default;

public:
    static const key_handle_pool& instance() { static const key_handle_pool pool; return pool; }

public:
    std::shared_ptr<key_handle_state> get(key_id id) const noexcept
    {
        using R = std::shared_ptr<key_handle_state>;

        switch (id) {
            case key_id::classes_root:                return R(m_pool, &m_pool->classes_root);
            case key_id::current_user:                return R(m_pool, &m_pool->current_user);
            case key_id::local_machine:               return R(m_pool, &m_pool->local_machine);
            case key_id::users:                       return R(m_pool, &m_pool->users);
            case key_id::performance_data:            return R(m_pool, &m_pool->performance_data);
            case key_id::performance_text:            return R(m_pool, &m_pool->performance_text);
            case key_id::performance_nlstext:         return R(m_pool, &m_pool->performance_nlstext);
            case key_id::current_config:              return R(m_pool, &m_pool->current_config);
            case key_id::current_user_local_settings: return R(m_pool, &m_pool->current_user_local_settings);
        }
        return R();
    }

private:
    const std::shared_ptr<pool_t> m_pool = std::make_shared<pool_t>();
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

uintmax_t remove_all_inside(const key& key, std::error_code& ec)
{
    /*
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

struct key_handle::state : key_handle_state { };

key_handle::key_handle(const weak_key_handle& handle)
    : m_state(handle.m_state.lock())
{
    if (handle.expired()) throw bad_weak_key_handle();
}

key_handle::key_handle(key_id id)
    : m_state(std::static_pointer_cast<state>(key_handle_pool::instance().get(id)))
{ }

key_handle::key_handle(native_handle_type handle, const registry::key& key, access_rights rights)
    : m_state(handle == native_handle_type{} ? nullptr :
        std::static_pointer_cast<state>(std::make_shared<key_handle_state>(key_handle_state{ handle, rights, key })))
{ }

key key_handle::key() const { return m_state ? m_state->key : registry::key(); }

access_rights key_handle::rights() const noexcept { return m_state ? m_state->rights : access_rights::unknown; }

key_handle::native_handle_type key_handle::native_handle() const noexcept
{
    return m_state ? static_cast<native_handle_type>(m_state->handle) : native_handle_type{};
}

bool key_handle::valid() const noexcept { return static_cast<bool>(m_state); }

std::pair<key_handle, bool> key_handle::create_key(const registry::key& subkey, 
                                                   access_rights rights, std::error_code& ec) const
{
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
    if (!RegDeleteKeyEx_) {
        rc = RegDeleteKey(reinterpret_cast<HKEY>(native_handle()), subkey.name().data());
    } else {
        rc = RegDeleteKeyEx_(reinterpret_cast<HKEY>(native_handle()),
                             subkey.name().data(), static_cast<DWORD>(subkey.view()), 0);
    }

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

std::uintmax_t key_handle::remove_all(const registry::key& subkey, std::error_code& ec) const
{
    // TODO: ...
    return 0;
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


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

key_handle open(const key& key, access_rights rights, std::error_code& ec)
{
    if (!key.is_absolute()) {
        const std::error_code ec2(ERROR_FILE_NOT_FOUND, std::system_category());
        return details::set_or_throw(&ec, ec2, __FUNCTION__, key), key_handle();
    }

    auto it = key.begin();
    const auto root = details::key_id_from_string(*it);
    const auto subkey = ++it != key.end() ? it->data() : TEXT("");

    LRESULT rc;
    key_handle::native_handle_type hkey;
    rc = RegOpenKeyEx(reinterpret_cast<HKEY>(root), subkey, 0,
                      static_cast<DWORD>(rights) | static_cast<DWORD>(key.view()), reinterpret_cast<HKEY*>(&hkey));

    if (rc == ERROR_SUCCESS) RETURN_RESULT(ec, key_handle(hkey, key, rights));

    const std::error_code ec2(rc, std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key), key_handle();
}

}  // namespace registry