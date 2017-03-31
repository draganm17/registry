#include <registry.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <locale>
#include <numeric>
#include <Windows.h>

#include <boost/algorithm/string.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/scope_exit.hpp>


namespace  {

using namespace registry;

//static_assert(sizeof(key) == sizeof(details::key_state), "");
//static_assert(sizeof(value) == sizeof(details::value_state), "");
//static_assert(sizeof(value_entry) == sizeof(details::value_entry_state), "");


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

string_view_type key_id_to_string(key_id id) noexcept
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

key_id key_id_from_string(string_view_type str) noexcept
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
    auto it = std::lower_bound(key_map.begin(), key_map.end(), str,
                               [](auto&& lhs, auto&& rhs) { return iequals(lhs.first, rhs); });

    return it != key_map.end() && (*it).first == str ? (*it).second : key_id::none;
}

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
//                             class registry_error                                   //
//------------------------------------------------------------------------------------//

struct registry_error::storage
{
    key          key1;
    key          key2;
    string_type  value_name;
};

registry_error::registry_error(std::error_code ec, const std::string& msg)
    : std::system_error(ec, msg)
{ }

registry_error::registry_error(std::error_code ec, const std::string& msg,
                               const key& key1)
    : std::system_error(ec, msg)
    , m_info(std::make_shared<storage>(storage{ key1 }))
{ }

registry_error::registry_error(std::error_code ec, const std::string& msg,
                               const key& key1, const key& key2)
    : std::system_error(ec, msg)
    , m_info(std::make_shared<storage>(storage{ key1, key2 }))
{ }

registry_error::registry_error(std::error_code ec, const std::string& msg,
                               const key& key1, const key& key2, string_view_type value_name)
    : std::system_error(ec, msg)
    , m_info(std::make_shared<storage>(storage{ key1, key2, static_cast<string_type>(value_name) }))
{ }

const key& registry_error::key1() const noexcept
{
    static const key empty_key;
    return m_info ? m_info->key1 : empty_key;
}

const key& registry_error::key2() const noexcept
{
    static const key empty_key;
    return m_info ? m_info->key2 : empty_key;
}

const string_type& registry_error::value_name() const noexcept
{
    static const string_type empty_value_name;
    return m_info ? m_info->value_name : empty_value_name;
}


//------------------------------------------------------------------------------------//
//                                   class key                                        //
//------------------------------------------------------------------------------------//

const view key::default_view =
#if defined(_WIN64)
    view::view_64bit;
#elif defined(_WIN32)
    view::view_32bit;
#endif

key key::from_key_id(key_id id) { return key(key_id_to_string(id)); }

key::key() noexcept
    : details::key_state{ default_view }
{ }

key::key(string_view_type name, registry::view view)
    : details::key_state{ view, static_cast<string_type>(name) }
{ }

const string_type& key::name() const noexcept { return m_name; }

view key::view() const noexcept { return m_view; }

key key::root_key() const { return has_root_key() ? key(*begin(), view()) : key(string_view_type(), view()); }

key key::leaf_key() const { return has_leaf_key() ? key(*--end(), view()) : key(string_view_type(), view()); }

key key::parent_key() const
{
    return has_parent_key() ? key(begin(), --end(), view()) : key(string_view_type(), view());
}

bool key::has_root_key() const noexcept { return !m_name.empty(); }

bool key::has_leaf_key() const noexcept { return !m_name.empty(); }

bool key::has_parent_key() const noexcept { return !m_name.empty() && ++begin() != end(); }

bool key::is_absolute() const noexcept { return !m_name.empty() && key_id_from_string(*begin()) != key_id::none; }

bool key::is_relative() const noexcept { return !is_absolute(); }

int key::compare(const key& other) const noexcept
{
    if (view() != other.view()) {
        return view() < other.view() ? -1 : 1;
    }

    iterator beg_1 = begin(), end_1 = end();
    iterator beg_2 = other.begin(), end_2 = other.end();
    for (; beg_1 != end_1 && beg_2 != end_2; ++beg_1, ++beg_2) {
        if (boost::ilexicographical_compare(*beg_1, *beg_2)) return -1;
        if (boost::ilexicographical_compare(*beg_2, *beg_1)) return  1;
    }
    return int(beg_2 == end_2) - int(beg_1 == end_1);
}

key::iterator key::begin() const noexcept
{
    size_t size = 0;
    for (; size < m_name.size() && m_name[size] == TEXT('\\'); ++size);
    for (; size < m_name.size() && m_name[size] != TEXT('\\'); ++size);
    return reinterpret_cast<iterator&&>(details::key_iterator_state{ m_name, { m_name.data(), size } });
}

key::iterator key::end() const noexcept
{
    return reinterpret_cast<iterator&&>(details::key_iterator_state{ m_name, { m_name.data() + m_name.size(), 0 } });
}

key& key::assign(string_view_type name, registry::view view)
{
    m_name.assign(name.data(), name.size());
    m_view = view;
    return *this;
}

key& key::append(string_view_type subkey)
{
    const bool add_slash = !(m_name.empty() || m_name.back() == TEXT('\\') ||
                             subkey.empty() || subkey.front() == TEXT('\\'));

    m_name.reserve(m_name.size() + subkey.size() + static_cast<int>(add_slash));
    m_name.append(add_slash ? TEXT("\\") : TEXT("")).append(subkey.data(), subkey.size());
    return *this;
}

key& key::concat(string_view_type subkey)
{
    m_name.append(subkey.data(), subkey.size());
    return *this;
}

key& key::remove_leaf()
{
    assert(has_leaf_key());

    auto it = --end();
    m_name.resize((it != begin()) ? (--it, it->data() - m_name.data() + it->size()) : 0);
    return *this;
}

key& key::replace_leaf(string_view_type replacement)
{
    assert(has_leaf_key());
    return remove_leaf().append(replacement);
}

void key::swap(key& other) noexcept
{
    using std::swap;
    swap(m_view, other.m_view);
    swap(m_name, other.m_name);
}

bool key::iterator::operator==(const iterator& rhs) const noexcept
{
    return m_value.data() == rhs.m_value.data() && m_value.size() == rhs.m_value.size();
}

bool key::iterator::operator!=(const iterator& rhs) const noexcept { return !(*this == rhs); }


//------------------------------------------------------------------------------------//
//                             class key::iterator                                    //
//------------------------------------------------------------------------------------//

key::iterator::reference key::iterator::operator*() const
{
    assert(!m_value.empty());
    return m_value;
}

key::iterator::pointer key::iterator::operator->() const
{
    assert(!m_value.empty());
    return &m_value;
}

key::iterator& key::iterator::operator++()
{
    assert(!m_value.empty());
    const auto end = m_key_string_view.end();

    auto first = m_value.end();
    for (; first != end && *first == TEXT('\\'); ++first);

    auto last = first;
    for (; last != end && *last != TEXT('\\'); ++last);

    m_value = string_view_type(first, last - first);
    return *this;
}

key::iterator key::iterator::operator++(int) { auto tmp = *this; ++*this; return tmp; }

key::iterator& key::iterator::operator--()
{
    // TODO: ...
    return *this;
}

key::iterator key::iterator::operator--(int) { auto tmp = *this; --*this; return tmp; }


//------------------------------------------------------------------------------------//
//                                 class value                                        //
//------------------------------------------------------------------------------------//

value::value(none_value_tag tag) noexcept
    : details::value_state{ value_type::none }
{ }

value::value(sz_value_tag tag, string_view_type value) { assign(tag, value); }

value::value(expand_sz_value_tag tag, string_view_type value) { assign(tag, value); }

value::value(binary_value_tag tag, byte_array_view_type value) { assign(tag, value); }

value::value(dword_value_tag tag, uint32_t value) { assign(tag, value); }

value::value(dword_big_endian_value_tag tag, uint32_t value) { assign(tag, value); }

value::value(link_value_tag tag, string_view_type value) { assign(tag, value); }

//value::value(multi_sz_value_tag tag, const std::vector<string_view_type>& value) { assign(tag, value); }

value::value(qword_value_tag tag, uint64_t value) { assign(tag, value); }

value::value(value_type type, byte_array_view_type data)
    : details::value_state{ type, { data.data(), data.data() + data.size() } }
{ }

value_type value::type() const noexcept { return m_type; }

byte_array_view_type value::data() const noexcept { return byte_array_view_type{ m_data.data(), m_data.size() }; }

uint32_t value::to_uint32() const
{
    uint8_t buf[sizeof(uint32_t)];
    memcpy(buf, m_data.data(), std::min(m_data.size(), sizeof(uint32_t)));

    using namespace boost::endian;
    switch (m_type) 
    {
        case value_type::dword:             return *reinterpret_cast<const uint32_t*>(buf);

        case value_type::dword_big_endian:  return static_cast<uint32_t>(*reinterpret_cast<const big_uint32_t*>(buf));
    }
    throw bad_value_cast();
}

uint64_t value::to_uint64() const
{
    uint8_t buf[sizeof(uint64_t)];
    memcpy(buf, m_data.data(), std::min(m_data.size(), sizeof(uint64_t)));

    using namespace boost::endian;
    switch (m_type) 
    {
        case value_type::dword:             return static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(buf));

        case value_type::dword_big_endian:  return static_cast<uint64_t>(*reinterpret_cast<const big_uint32_t*>(buf));

        case value_type::qword:             return *reinterpret_cast<const uint64_t*>(buf);
    }
    throw bad_value_cast();
}

string_type value::to_string() const
{
    if (m_type == value_type::sz || m_type == value_type::expand_sz || m_type == value_type::link)
    {
        const auto chars = m_data.size() / sizeof(string_type::value_type);
        const auto str_ptr = reinterpret_cast<const string_type::value_type*>(m_data.data());
        return chars ? string_type(str_ptr, chars - (str_ptr[chars - 1] ? 0 : 1)) : string_type{};
    }
    throw bad_value_cast();
}

std::vector<string_type> value::to_strings() const
{
    if (m_type == value_type::multi_sz)
    {
        std::vector<string_type> result;
        const auto chars = m_data.size() / sizeof(string_type::value_type);
        const auto str_ptr = reinterpret_cast<const string_type::value_type*>(m_data.data());
        
        auto first = str_ptr, last = str_ptr, end = &str_ptr[chars];
        while (last != end) {
            if (!(*last++) || last == end) {
                if (!(last == end && last - first == 1 && *first == TEXT('\0'))) {
                    result.emplace_back(first, last - (last == end ? 0 : 1));
                }
                first = last;
            }
        }
        return result;
    }
    throw bad_value_cast();
}

byte_array_type value::to_byte_array() const
{
    return m_type == value_type::binary ? m_data : throw bad_value_cast();
}

value& value::assign(none_value_tag) noexcept
{
    m_data.clear();
    m_type = value_type::none;

    return *this;
}

value& value::assign(sz_value_tag, string_view_type value)
{
    static constexpr auto null_teminator = TEXT('\0');
    const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
    const auto data_size = value.size() * sizeof(string_view_type::value_type);

    m_data.resize(data_size + sizeof(null_teminator));
    m_type = value_type::sz;

    memcpy(m_data.data(), data_ptr, data_size);
    memcpy(m_data.data() + data_size, &null_teminator, sizeof(null_teminator));

    return *this;
}

value& value::assign(expand_sz_value_tag, string_view_type value)
{
    static constexpr auto null_teminator = TEXT('\0');
    const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
    const auto data_size = value.size() * sizeof(string_view_type::value_type);

    m_data.resize(data_size + sizeof(null_teminator));
    m_type = value_type::expand_sz;

    memcpy(m_data.data(), data_ptr, data_size);
    memcpy(m_data.data() + data_size, &null_teminator, sizeof(null_teminator));

    return *this;
}

value& value::assign(binary_value_tag, byte_array_view_type value)
{
    m_data.resize(value.size());
    m_type = value_type::binary;
    memcpy(m_data.data(), value.data(), value.size());

    return *this;
}

value& value::assign(dword_value_tag, uint32_t value)
{
    m_data.resize(sizeof(uint32_t));
    m_type = value_type::dword;
    memcpy(m_data.data(), &value, sizeof(uint32_t));

    return *this;
}

value& value::assign(dword_big_endian_value_tag, uint32_t value)
{
    m_data.resize(sizeof(uint32_t));
    m_type = value_type::dword_big_endian;

    boost::endian::big_uint32_t value_copy = value;
    memcpy(m_data.data(), value_copy.data(), sizeof(uint32_t));

    return *this;
}

value& value::assign(link_value_tag, string_view_type value)
{
    static constexpr auto null_teminator = TEXT('\0');
    const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
    const auto data_size = value.size() * sizeof(string_view_type::value_type);

    m_data.resize(data_size + sizeof(null_teminator));
    m_type = value_type::link;

    memcpy(m_data.data(), data_ptr, data_size);
    memcpy(m_data.data() + data_size, &null_teminator, sizeof(null_teminator));

    return *this;
}

value& value::assign(multi_sz_value_tag, const std::vector<string_view_type>& value)
{
    static constexpr auto null_teminator = TEXT('\0');

    const auto buffer_size = std::accumulate(value.begin(), value.end(), size_t(0), [](size_t sz, const auto& value) {
        return sz + value.size() * sizeof(string_type::value_type) + sizeof(null_teminator);
    }) + sizeof(null_teminator);

    m_data.resize(buffer_size);
    m_type = value_type::multi_sz;

    size_t offset = 0;
    std::for_each(value.begin(), value.end(), [&](const auto& value) noexcept
    {
        const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
        const auto data_size = value.size() * sizeof(string_view_type::value_type);

        memcpy(m_data.data() + offset, data_ptr, data_size);
        memcpy(m_data.data() + data_size + offset, &null_teminator, sizeof(null_teminator));
        offset += data_size + sizeof(null_teminator);
    });
    memcpy(m_data.data() + offset, &null_teminator, sizeof(null_teminator));

    return *this;
}
/*
value& value::assign_impl(multi_sz_value_tag, const std::function<bool(string_view_type&)>& enumerator)
{
    string_view_type value;
    auto enumerator_copy = enumerator;
    size_t offset = 0, buffer_size = 0;
    static constexpr auto null_teminator = TEXT('\0');

    while (enumerator(value)) {
        buffer_size += value.size() * sizeof(string_type::value_type) + sizeof(null_teminator);
    }

    m_data.resize(buffer_size);
    m_type = value_type::multi_sz;
    while (enumerator_copy(value))
    {
        const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
        const auto data_size = value.size() * sizeof(string_view_type::value_type);

        memcpy(m_data.data() + offset, data_ptr, data_size);
        memcpy(m_data.data() + data_size + offset, &null_teminator, sizeof(null_teminator));
        offset += data_size + sizeof(null_teminator);
    }
    memcpy(m_data.data() + offset, &null_teminator, sizeof(null_teminator));

    return *this;
}*/

value& value::assign(qword_value_tag, uint64_t value)
{
    m_data.resize(sizeof(uint64_t));
    m_type = value_type::qword;
    memcpy(m_data.data(), &value, sizeof(uint64_t));

    return *this;
}

void value::swap(value& other) noexcept
{
    using std::swap;
    swap(m_type, other.m_type);
    swap(m_data, other.m_data);
}


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
{ return m_state ? static_cast<native_handle_type>(m_state->handle) : native_handle_type(); }

bool key_handle::valid() const noexcept { return static_cast<bool>(m_state); }

bool key_handle::exists(string_view_type value_name)
{
    std::error_code ec;
    decltype(auto) res = exists(value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key(), {}, value_name);
    return res;
}

bool key_handle::exists(string_view_type value_name, std::error_code& ec)
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

    return !ec ? info : key_info();
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


//------------------------------------------------------------------------------------//
//                               class key_entry                                      //
//------------------------------------------------------------------------------------//

key_entry::key_entry(const registry::key& key)
    : m_key(key)
{ }

key_entry::key_entry(const key_handle& handle)
    : m_key(handle.key())
    , m_key_handle(handle)
{ }

const registry::key& key_entry::key() const noexcept { return m_key; }

key_info key_entry::info(key_info_mask mask) const
{
    std::error_code ec;
    decltype(auto) res = info(mask, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, m_key);
    return res;
}

key_info key_entry::info(key_info_mask mask, std::error_code& ec) const
{
    ec.clear();
    auto handle = m_key_handle.lock();
    return handle.valid() ? handle.info(mask, ec) : registry::info(m_key, mask, ec);
}

key_entry& key_entry::assign(const registry::key& key)
{ 
    m_key = key;
    m_key_handle.swap(weak_key_handle());
    return *this;
}

key_entry& key_entry::assign(const key_handle& handle)
{
    m_key.swap(registry::key());
    m_key_handle = handle;
    return *this;
}

void key_entry::swap(key_entry& other) noexcept { m_key.swap(other.m_key); }


//------------------------------------------------------------------------------------//
//                              class value_entry                                     //
//------------------------------------------------------------------------------------//

value_entry::value_entry(const registry::key& key, string_view_type value_name)
    : m_key(key)
    , m_value_name(value_name)
{ }

value_entry::value_entry(const registry::key_handle& handle, string_view_type value_name)
    : m_key(handle.key())
    , m_value_name(value_name)
    , m_key_handle(handle)
{ }

const key& value_entry::key() const noexcept { return m_key; }

const string_type& value_entry::value_name() const noexcept { return m_value_name; }

value value_entry::value() const
{
    std::error_code ec;
    decltype(auto) res = value(ec);
    if (ec) throw registry_error(ec, __FUNCTION__, m_key, {}, m_value_name);
    return res;
}

value value_entry::value(std::error_code& ec) const
{
    ec.clear();
    auto handle = m_key_handle.lock();
    return handle.valid() ? handle.read_value(m_value_name, ec) : registry::read_value(m_key, m_value_name, ec);
}

value_entry& value_entry::assign(const registry::key& key, string_view_type value_name)
{
    m_key = key;
    m_value_name.assign(value_name.data(), value_name.size());
    m_key_handle.swap(weak_key_handle());
    return *this;
}

value_entry& value_entry::assign(const key_handle& handle, string_view_type value_name)
{
    m_key.swap(registry::key());
    m_value_name.assign(value_name.data(), value_name.size());
    m_key_handle = handle;
    return *this;
}

void value_entry::swap(value_entry& other) noexcept
{
    using std::swap;
    swap(m_key, other.m_key);
    swap(m_value_name, other.m_value_name);
}


//------------------------------------------------------------------------------------//
//                              class key_iterator                                    //
//------------------------------------------------------------------------------------//

key_iterator::key_iterator(const key& key)
{
    std::error_code ec;
    auto tmp = key_iterator(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    swap(tmp);
}

key_iterator::key_iterator(const key& key, std::error_code& ec)
{
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) {
        if (ec) swap(key_iterator());
        if (ec.value() == ERROR_FILE_NOT_FOUND) ec.clear();
    };

    auto handle = open(key, access_rights::enumerate_sub_keys, ec);
    if (!ec) swap(key_iterator(handle, ec));
}

key_iterator::key_iterator(const key_handle& handle)
{
    std::error_code ec;
    auto tmp = key_iterator(handle, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, handle.key());
    swap(tmp);
}

key_iterator::key_iterator(const key_handle& handle, std::error_code& ec)
    : m_idx(-1)
    , m_hkey(handle)
    , m_entry(handle)
{
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) swap(key_iterator()); };
    key_info info = handle.info(key_info_mask::read_max_subkey_size, ec);

    if (!ec) {
        m_buffer.resize(++info.max_subkey_size, TEXT('_'));
        m_entry.m_key.append({ m_buffer.data(), m_buffer.size() });
        increment(ec);
    }
}

bool key_iterator::operator==(const key_iterator& rhs) const noexcept
{
    // TODO: ...
    return 0;

    //return (!m_state || !rhs.m_state) ? (!m_state && !rhs.m_state)
    //                                  : (reinterpret_cast<key&>(m_state->key_state) == 
    //                                     reinterpret_cast<key&>(rhs.m_state->key_state));
}

bool key_iterator::operator!=(const key_iterator& rhs) const noexcept { return !(*this == rhs); }

key_iterator::reference key_iterator::operator*() const
{
    assert(*this != key_iterator());
    return m_entry;
}

key_iterator::pointer key_iterator::operator->() const
{
    assert(*this != key_iterator());
    return &m_entry;
}

key_iterator& key_iterator::operator++()
{
    std::error_code ec;
    decltype(auto) res = increment(ec);
    if (ec) throw registry_error(ec, __FUNCTION__);
    return res;
}

key_iterator key_iterator::operator++(int) { auto tmp = *this; ++*this; return tmp; }

key_iterator& key_iterator::increment(std::error_code& ec)
{
    assert(*this != key_iterator());

    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) {
        if (ec) swap(key_iterator());
        if (ec.value() == ERROR_NO_MORE_ITEMS) ec.clear();
    };

    // NOTE: Subkeys which names size exceed the size of the pre-allocated buffer are ignored.
    //       Such values may only appear in the enumerated sequence if they were added to the registry key after the
    //       iterator was constructed. Therefore this behaviour is consistent with what the class documentation states.

    do {
        DWORD buffer_size = m_buffer.size();
        HKEY hkey = reinterpret_cast<HKEY>(m_hkey.native_handle());
        LSTATUS rc = RegEnumKeyEx(hkey, ++m_idx, m_buffer.data(), &buffer_size, nullptr, nullptr, nullptr, nullptr);

        if (!(ec = std::error_code(rc, std::system_category()))) {
            m_entry.m_key.replace_leaf({ m_buffer.data(), buffer_size });
        }
    } while (ec.value() == ERROR_MORE_DATA);

    return *this;
}

void key_iterator::swap(key_iterator& other) noexcept
{
    using std::swap;
    swap(m_idx, other.m_idx);
    swap(m_hkey, other.m_hkey);
    swap(m_entry, other.m_entry);
    swap(m_buffer, other.m_buffer);
}


//------------------------------------------------------------------------------------//
//                         class recursive_key_iterator                               //
//------------------------------------------------------------------------------------//

recursive_key_iterator::recursive_key_iterator(const key& key)
{
    std::error_code ec;
    auto tmp = recursive_key_iterator(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    swap(tmp);
}

recursive_key_iterator::recursive_key_iterator(const key& key, std::error_code& ec)
{
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) { if (ec || (!m_stack.empty() && m_stack.back() == key_iterator())) m_stack.clear(); };

    m_stack.emplace_back(key, ec);
}

recursive_key_iterator::recursive_key_iterator(const key_handle& handle)
{
    std::error_code ec;
    auto tmp = recursive_key_iterator(handle, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, handle.key());
    swap(tmp);
}

recursive_key_iterator::recursive_key_iterator(const key_handle& handle, std::error_code& ec)
{
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) m_stack.clear(); };

    m_stack.emplace_back(handle, ec);
}

bool recursive_key_iterator::operator==(const recursive_key_iterator& rhs) const noexcept
{
    return (m_stack.empty() || rhs.m_stack.empty()) ? (m_stack.empty() && rhs.m_stack.empty())
                                                    : (m_stack.back() == rhs.m_stack.back());
}

bool recursive_key_iterator::operator!=(const recursive_key_iterator& rhs) const noexcept { return !(*this == rhs); }

recursive_key_iterator::reference recursive_key_iterator::operator*() const
{
    assert(*this != recursive_key_iterator());
    return *m_stack.back();
}

recursive_key_iterator::pointer recursive_key_iterator::operator->() const
{
    assert(*this != recursive_key_iterator());
    return m_stack.back().operator->();
}

int recursive_key_iterator::depth() const
{
    assert(*this != recursive_key_iterator());
    return static_cast<int>(m_stack.size() - 1);
}

recursive_key_iterator& recursive_key_iterator::operator++()
{
    std::error_code ec;
    decltype(auto) res = increment(ec);
    if (ec) throw registry_error(ec, __FUNCTION__);
    return res;
}

recursive_key_iterator recursive_key_iterator::operator++(int) { auto tmp = *this; ++*this; return tmp; }

recursive_key_iterator& recursive_key_iterator::increment(std::error_code& ec)
{
    assert(*this != recursive_key_iterator());
    
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) m_stack.clear(); };

    m_stack.emplace_back(m_stack.back()->key(), ec);
    if (m_stack.back() == key_iterator()) 
    {
        do {
            m_stack.pop_back();
            if (m_stack.size()) {
                m_stack.back().increment(ec);
            }
        } while (!ec && !m_stack.empty() && m_stack.back() == key_iterator());
    }
    return *this;
}

void recursive_key_iterator::pop()
{
    assert(*this != recursive_key_iterator());
    m_stack.pop_back();
}

void recursive_key_iterator::swap(recursive_key_iterator& other) noexcept { m_stack.swap(other.m_stack); }


//------------------------------------------------------------------------------------//
//                             class value_iterator                                   //
//------------------------------------------------------------------------------------//

value_iterator::value_iterator(const key& key)
{
    std::error_code ec;
    auto tmp = value_iterator(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    swap(tmp);
}

value_iterator::value_iterator(const key& key, std::error_code& ec)
{
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) {
        if (ec) swap(value_iterator());
        if (ec.value() == ERROR_FILE_NOT_FOUND) ec.clear();
    };

    auto handle = open(key, access_rights::query_value, ec);
    if (!ec) swap(value_iterator(handle, ec));
}

value_iterator::value_iterator(const key_handle& handle)
{
    std::error_code ec;
    auto tmp = value_iterator(handle, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, handle.key());
    swap(tmp);
}

value_iterator::value_iterator(const key_handle& handle, std::error_code& ec)
    : m_idx(-1)
    , m_hkey(handle)
    , m_entry(handle, string_type())
{
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) swap(value_iterator()); };
    key_info info = handle.info(key_info_mask::read_max_value_name_size, ec);

    if (!ec) {
        m_buffer.resize(++info.max_value_name_size);
        m_entry.m_value_name.reserve(info.max_value_name_size);
        increment(ec);
    }
}

bool value_iterator::operator==(const value_iterator& rhs) const noexcept
{
    // TODO: ...
    return 0;

    //return (!m_state || !rhs.m_state) ? (!m_state && !rhs.m_state)
    //                                  : (reinterpret_cast<value_entry&>(m_state->entry_state) ==
    //                                     reinterpret_cast<value_entry&>(rhs.m_state->entry_state));
}

bool value_iterator::operator!=(const value_iterator& rhs) const noexcept { return !(*this == rhs); }

value_iterator::reference value_iterator::operator*() const
{
    assert(*this != value_iterator());
    return m_entry;
}

value_iterator::pointer value_iterator::operator->() const
{
    assert(*this != value_iterator());
    return &m_entry;
}

value_iterator& value_iterator::operator++()
{
    std::error_code ec;
    decltype(auto) res = increment(ec);
    if (ec) throw registry_error(ec, __FUNCTION__);
    return res;
}

value_iterator value_iterator::operator++(int) { auto tmp = *this; ++*this; return tmp; }

value_iterator& value_iterator::increment(std::error_code& ec)
{
    assert(*this != value_iterator());

    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) {
        if (ec) swap(value_iterator());
        if (ec.value() == ERROR_NO_MORE_ITEMS) ec.clear();
    };

    // NOTE: Values which names size exceed the size of the pre-allocated buffer are ignored.
    //       Such values may only appear in the enumerated sequence if they were added to the registry key after the
    //       iterator was constructed. Therefore this behaviour is consistent with what the class documentation states.

    do {
        DWORD buffer_size = m_buffer.size();
        HKEY hkey = reinterpret_cast<HKEY>(m_hkey.native_handle());
        LSTATUS rc = RegEnumValue(hkey, ++m_idx, m_buffer.data(), &buffer_size, nullptr, nullptr, nullptr, nullptr);

        if (!(ec = std::error_code(rc, std::system_category()))) {
            m_entry.m_value_name.assign(m_buffer.data(), buffer_size);
        }
    } while (ec.value() == ERROR_MORE_DATA);

    return *this;
}

void value_iterator::swap(value_iterator& other) noexcept 
{ 
    using std::swap;
    swap(m_idx, other.m_idx);
    swap(m_hkey, other.m_hkey);
    swap(m_entry, other.m_entry);
    swap(m_buffer, other.m_buffer);
}


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

key_handle open(const key& key, access_rights rights)
{
    std::error_code ec;
    decltype(auto) res = open(key, rights, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

key_handle open(const key& key, access_rights rights, std::error_code& ec)
{
    ec.clear();
    if (!key.is_absolute()) {
        ec = std::error_code(ERROR_FILE_NOT_FOUND, std::system_category());
        return key_handle();
    }

    auto it = key.begin();
    const auto root = key_id_from_string(*it);
    const auto subkey = ++it != key.end() ? it->data() : TEXT("");

    LRESULT rc;
    key_handle::native_handle_type hkey;
    rc = RegOpenKeyEx(reinterpret_cast<HKEY>(root), subkey, 0,
                      static_cast<DWORD>(rights) | static_cast<DWORD>(key.view()), reinterpret_cast<HKEY*>(&hkey));

    return (rc == ERROR_SUCCESS) ? key_handle(hkey, key, rights)
                                 : (ec = std::error_code(rc, std::system_category()), key_handle());
}

bool exists(const key& key)
{
   std::error_code ec;
   decltype(auto) res = exists(key, ec);
   if (ec) throw registry_error(ec, __FUNCTION__, key);
   return res;
}

bool exists(const key& key, std::error_code& ec)
{
    ec.clear();
    open(key, access_rights::query_value, ec);
    return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), false)
                                                : (!ec ? true : false);
}

bool exists(const key& key, string_view_type value_name)
{
    std::error_code ec;
    decltype(auto) res = exists(key, value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, value_name);
    return res;
}

bool exists(const key& key, string_view_type value_name, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::query_value, ec);
    return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), false)
                                                : (!ec ? handle.exists(value_name, ec) : false);
}

key_info info(const key& key, key_info_mask mask)
{
    std::error_code ec;
    decltype(auto) res = info(key, mask, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

key_info info(const key& key, key_info_mask mask, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::query_value, ec);
    return !ec ? handle.info(mask, ec) : key_info();
}

value read_value(const key& key, string_view_type value_name)
{
    std::error_code ec;
    decltype(auto) res = read_value(key, value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, value_name);
    return res;
}

value read_value(const key& key, string_view_type value_name, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::query_value, ec);
    return !ec ? handle.read_value(value_name, ec) : value();
}

bool create_key(const key& key)
{
    std::error_code ec;
    decltype(auto) res = create_key(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

bool create_key(const key& key, std::error_code& ec)
{
    ec.clear();
    registry::key base_key = key, subkey;
    key_handle handle = open(base_key, access_rights::create_sub_key, ec);

    while ((!ec || ec.value() == ERROR_FILE_NOT_FOUND) && base_key.has_parent_key()) {
        subkey = base_key.leaf_key().append(subkey.name());
        handle = open(base_key.remove_leaf(), access_rights::create_sub_key, ec);
    }
    return !ec ? handle.create_key(subkey, access_rights::query_value, ec).second : false;
}

void write_value(const key& key, string_view_type value_name, const value& value)
{
    std::error_code ec;
    write_value(key, value_name, value, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, value_name);
}

void write_value(const key& key, string_view_type value_name, const value& value, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::set_value, ec);
    if (!ec) handle.write_value(value_name, value, ec);
}

bool remove(const key& key)
{
    std::error_code ec;
    decltype(auto) res = remove(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

bool remove(const key& key, std::error_code& ec)
{
    // TODO: key open rights does not affect the delete operation - 
    //       so, maybe I should 'open' just the root key and not the parent key ???

    ec.clear();
    auto handle = open(key.parent_key(), access_rights::query_value /* TODO: ??? */, ec);

    return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), false)
                                                : (!ec ? handle.remove(key.leaf_key(), ec) : false);

    // TODO: check if the key has a parent ???
}

bool remove(const key& key, string_view_type value_name)
{
    std::error_code ec;
    decltype(auto) res = remove(key, value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, value_name);
    return res;
}

bool remove(const key& key, string_view_type value_name, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::set_value, ec);
    return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), false)
                                                : (!ec ? handle.remove(value_name, ec) : false);
}

std::uintmax_t remove_all(const key& key)
{
    std::error_code ec;
    decltype(auto) res = remove_all(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

std::uintmax_t remove_all(const key& key, std::error_code& ec)
{
    // TODO: ...

    ec.clear();

    auto keys_deleted = remove_all_inside(key, ec);
    if (!ec) {
        keys_deleted += remove(key, ec) ? 1 : 0;
    }
    return ec ? static_cast<std::uintmax_t>(-1) : keys_deleted;
}

bool equivalent(const key& key1, const key& key2)
{
    std::error_code ec;
    decltype(auto) res = equivalent(key1, key2, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key1, key2);
    return res;
}

bool equivalent(const key& key1, const key& key2, std::error_code& ec)
{
    ec.clear();
    auto handle1 = open(key1, access_rights::query_value, ec);
    auto handle2 = !ec ? open(key2, access_rights::query_value, ec) : key_handle();

    return !ec ? handle1.equivalent(handle2, ec) : false;
}

}  // namespace registry