#include <registry.h>

#include <algorithm>
#include <cassert>
#include <locale>
#include <numeric>
#include <Windows.h>

#include <boost/algorithm/string.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/scope_exit.hpp>


namespace  {

using namespace registry;

static_assert(sizeof(key) == sizeof(details::key_state), "");
static_assert(sizeof(value) == sizeof(details::value_state), "");
static_assert(sizeof(value_entry) == sizeof(details::value_entry_state), "");

struct smart_hkey_deleter
{
    void operator()(void* hkey) const noexcept
    {
        static_assert(WINVER <= 0x0A00, "Unsupported Windows version");

        switch ((ULONG_PTR)hkey) {
            case (ULONG_PTR)0x0                              :
            case (ULONG_PTR)HKEY_CLASSES_ROOT                :
            case (ULONG_PTR)HKEY_CURRENT_USER                :
            case (ULONG_PTR)HKEY_LOCAL_MACHINE               :
            case (ULONG_PTR)HKEY_USERS                       :
            case (ULONG_PTR)HKEY_PERFORMANCE_DATA            :
            case (ULONG_PTR)HKEY_PERFORMANCE_TEXT            :
            case (ULONG_PTR)HKEY_PERFORMANCE_NLSTEXT         :
            case (ULONG_PTR)HKEY_CURRENT_CONFIG              :
            case (ULONG_PTR)HKEY_DYN_DATA                    :
            case (ULONG_PTR)HKEY_CURRENT_USER_LOCAL_SETTINGS : return;
        }
        ::RegCloseKey((HKEY)hkey);
    }
};

template <typename SmartPtr>
class smart_hkey_base
{
    static_assert(std::is_same<SmartPtr, std::unique_ptr<void, smart_hkey_deleter>>::value ||
                  std::is_same<SmartPtr, std::shared_ptr<void>>::value, "Invalid type for SmartPtr");

    SmartPtr m_hkey;

public:
    constexpr smart_hkey_base() noexcept 
    : m_hkey(nullptr)
    { }

    constexpr smart_hkey_base(HKEY hkey) 
        noexcept(std::is_nothrow_constructible<SmartPtr, void*, smart_hkey_deleter>::value)
    : m_hkey((void*)hkey, smart_hkey_deleter{})
    { }

public:
    operator HKEY() const noexcept { return (HKEY)m_hkey.get(); }

    HKEY operator*() const noexcept { return (HKEY)m_hkey.get(); }

    explicit operator bool() const noexcept { return static_cast<bool>(m_hkey); }
};

using shared_hkey = smart_hkey_base<std::shared_ptr<void>>;
using unique_hkey = smart_hkey_base<std::unique_ptr<void, smart_hkey_deleter>>;


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
        case registry::key_id::classes_root :                return TEXT("HKEY_CLASSES_ROOT");
        case registry::key_id::current_user :                return TEXT("HKEY_CURRENT_USER");
        case registry::key_id::local_machine :               return TEXT("HKEY_LOCAL_MACHINE");
        case registry::key_id::users :                       return TEXT("HKEY_USERS");
        case registry::key_id::performance_data :            return TEXT("HKEY_PERFORMANCE_DATA");
        case registry::key_id::current_config :              return TEXT("HKEY_CURRENT_CONFIG");
        case registry::key_id::dyn_data :                    return TEXT("HKEY_DYN_DATA");
        case registry::key_id::current_user_local_settings : return TEXT("HKEY_CURRENT_USER_LOCAL_SETTINGS");
        default :                                            return TEXT("");
    };
}

key_id key_id_from_string(string_view_type str) noexcept
{
    using namespace boost;
    if (iequals(str, TEXT("HKEY_CLASSES_ROOT")))                 return registry::key_id::classes_root;
    if (iequals(str, TEXT("HKEY_CURRENT_USER")))                 return registry::key_id::current_user;
    if (iequals(str, TEXT("HKEY_LOCAL_MACHINE")))                return registry::key_id::local_machine;
    if (iequals(str, TEXT("HKEY_USERS")))                        return registry::key_id::users;
    if (iequals(str, TEXT("HKEY_PERFORMANCE_DATA")))             return registry::key_id::performance_data;
    if (iequals(str, TEXT("HKEY_CURRENT_CONFIG")))               return registry::key_id::current_config;
    if (iequals(str, TEXT("HKEY_DYN_DATA")))                     return registry::key_id::dyn_data;
    if (iequals(str, TEXT("HKEY_CURRENT_USER_LOCAL_SETTINGS")))  return registry::key_id::current_user_local_settings;
    return static_cast<registry::key_id>(-1);
}

std::uintmax_t remove_all_inside(const key& key, std::error_code& ec)
{
    ec.clear();
    if (key.empty()) {
        return 0;
    }

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
}

time_t file_time_to_time_t(const FILETIME time) noexcept
{
    const uint64_t t = (static_cast<uint64_t>(time.dwHighDateTime) << 32) | time.dwLowDateTime;
    return static_cast<time_t>((t - 116444736000000000ll) / 10000000);
}

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

key::key() noexcept
    : details::key_state{ default_view, static_cast<key_id>(-1) }
{ }

key::key(key_id root, registry::view view)
    : details::key_state{ view, root, static_cast<string_type>(key_id_to_string(root)) }
{ }

key::key(string_view_type name, registry::view view)
    : details::key_state{ view, 
                          key_id_from_string(name.substr(0, name.find(TEXT('\\')))), 
                          static_cast<string_type>(name) }
{
    if (static_cast<std::underlying_type_t<key_id>>(m_root) == -1) throw bad_key_name();
}

key::key(key_id root, string_view_type subkey, registry::view view)
    : details::key_state{ view, 
                          root, 
                          static_cast<string_type>(key_id_to_string(root)) + 
                              (subkey.empty() ? TEXT("") : (TEXT("\\") + static_cast<string_type>(subkey))) }
{ }

key& key::operator=(const key& other)
{
    if (this != &other) {
        swap(key(other));
    }
    return *this;
}

int key::compare(const key& other) const noexcept
{
    if (empty() || other.empty()) return (int)other.empty() - (int)empty();
    if (view() != other.view())   return view() < other.view() ? -1 : 1;
    if (root() != other.root())   return root() < other.root() ? -1 : 1;

    iterator beg_1 = ++begin(), end_1 = end();
    iterator beg_2 = ++other.begin(), end_2 = other.end();
    for (; beg_1 != end_1 && beg_2 != end_2; ++beg_1, ++beg_2) {
        if (boost::ilexicographical_compare(*beg_1, *beg_2)) return -1;
        if (boost::ilexicographical_compare(*beg_2, *beg_1)) return  1;
    }
    return int(beg_2 == end_2) - int(beg_1 == end_1);
}

key_id key::root() const
{
    assert(empty() == false);
    return m_root;
}

string_view_type key::name() const
{
    assert(empty() == false);
    return m_name;
}

view key::view() const
{
    assert(empty() == false);
    return m_view;
}

string_view_type key::subkey() const
{
    assert(empty() == false);
    auto pos = m_name.find(TEXT('\\'));
    for (; pos < m_name.size() && m_name[pos] == TEXT('\\'); ++pos);
    return (pos >= m_name.size()) ? string_view_type{} 
                                  : string_view_type{ m_name.data() + pos, m_name.size() - pos };
}

bool key::has_subkey() const
{
    assert(empty() == false);
    return !subkey().empty();
}

key key::parent_key() const
{
    assert(empty() == false);
    return has_parent_key() ? key(*this).remove_subkey() : key();
}

bool key::has_parent_key() const
{
    assert(empty() == false);
    return has_subkey();
}

bool key::empty() const noexcept { return static_cast<std::underlying_type_t<key_id>>(m_root) == -1; }

key::iterator key::begin() const noexcept
{
    if (empty()) {
        return end();
    }

    iterator it;
    it.m_key_string_view = m_name;
    it.m_value = string_view_type(m_name.data(), std::min(m_name.find(TEXT("\\")), m_name.size()));
    return it;
}

key::iterator key::end() const noexcept
{
    iterator it;
    it.m_key_string_view = m_name;
    it.m_value = string_view_type(m_name.data() + m_name.size(), 0);
    return it;
}

key& key::assign(key_id root, registry::view view)
{
    return empty() ? (swap(key(root, view)), *this)
                   : (has_subkey() ? remove_subkey() : *this).replace_root(root).replace_view(view);
}

key& key::assign(string_view_type name, registry::view view)
{
    if (empty()) {
        return swap(key(name, view)), *this;
    }

    const auto new_root_id = key_id_from_string(name.substr(0, name.find(TEXT('\\'))));
    if (static_cast<std::underlying_type_t<key_id>>(new_root_id) == -1) throw bad_key_name();

    m_name.assign(name.data(), name.size());
    m_root = new_root_id;
    m_view = view;
    return *this;
}

key& key::assign(key_id root, string_view_type subkey, registry::view view)
{
    return empty() ? (swap(key(root, subkey, view)), *this)
                   : (has_subkey() ? replace_subkey(subkey) : *this).replace_root(root).replace_view(view);
}

key& key::append(string_view_type subkey)
{
    assert(empty() == false);
    m_name.append(m_name.back() == TEXT('\\') ? TEXT("") : TEXT("\\"));
    m_name.append(subkey.data(), subkey.size());
    return *this;
}

key& key::replace_root(key_id id)
{
    assert(empty() == false);
    if (id != m_root) {
        const auto new_root_str = key_id_to_string(id);
        const auto old_root_str = key_id_to_string(root());
        m_name.replace(0, old_root_str.size(), new_root_str.data(), new_root_str.size());
        m_root = id;
    }
    return *this;
}

key& key::replace_subkey(string_view_type subkey)
{
    assert(empty() == false);
    assert(has_subkey() == true);
    const auto old_subkey = this->subkey();
    m_name.replace(old_subkey.data() - m_name.data(), old_subkey.size(), subkey.data(), subkey.size());
    return *this;
}

key& key::replace_view(registry::view view)
{
    assert(empty() == false);
    m_view = view;
    return *this;
}

key& key::remove_subkey()
{
    assert(empty() == false);
    assert(has_subkey() == true);
    m_name.resize(m_name.size() - subkey().size() - 1);
    return *this;
}

void key::clear() noexcept { m_root = static_cast<key_id>(-1); }

void key::swap(key& other) noexcept
{
    std::swap(m_view, other.m_view);
    std::swap(m_root, other.m_root);
    std::swap(m_name, other.m_name);
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
        while (last != end)
        {
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
    std::swap(m_type, other.m_type);
    std::swap(m_data, other.m_data);
}


//------------------------------------------------------------------------------------//
//                              class value_entry                                     //
//------------------------------------------------------------------------------------//

value_entry::value_entry(const registry::key& key, string_view_type value_name)
    : value_entry_state{ key, static_cast<string_type>(value_name) }
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

value value_entry::value(std::error_code& ec) const { return read_value(m_key, m_value_name, ec); }

void value_entry::swap(value_entry& other) noexcept
{
    std::swap(m_key, other.m_key);
    std::swap(m_value_name, other.m_value_name);
}


//------------------------------------------------------------------------------------//
//                              class key_iterator                                    //
//------------------------------------------------------------------------------------//

struct key_iterator::state
{
    shared_hkey         hkey;
    uint32_t            key_idx;
    uint32_t            subkey_pos;
    details::key_state  key_state;
};

key_iterator::key_iterator() noexcept = default;

key_iterator::key_iterator(const key_iterator& other)
    : m_state(other.m_state ? std::make_unique<state>(*other.m_state) : nullptr)
{ }

key_iterator::key_iterator(key_iterator&& other) noexcept = default;

key_iterator& key_iterator::operator=(const key_iterator& other)
{
    if (this != &other) {
        swap(key_iterator(other));
    }
    return *this;
}

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
    if (key.empty()) {
        return; //return end iterator
    }
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) m_state.reset(); };

    HKEY hkey{};
    LSTATUS rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 0,
                              KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | static_cast<DWORD>(key.view()), &hkey);
    if (rc != ERROR_SUCCESS) {
        ec = (rc == ERROR_FILE_NOT_FOUND) ? std::error_code{} : std::error_code(rc, std::system_category());
        return; //return end iterator
    }

    //NOTE: hkey will not leak, its ownership is thranfered before any other operation.
    m_state = std::make_unique<state>(state{
        hkey,
        uint32_t(-1),
        uint32_t(key.name().size() + (key.name().back() == TEXT('\\') ? 0 : 1)),
        details::key_state {
            key.view(),
            key.root(),
            static_cast<string_type>(key.name()).append(key.name().back() == TEXT('\\') ? TEXT("") : TEXT("\\"))
        }
    });
    increment(ec);
}

key_iterator::~key_iterator() = default;

key_iterator& key_iterator::operator=(key_iterator&& other) noexcept = default;

bool key_iterator::operator==(const key_iterator& rhs) const noexcept
{
    return (!m_state || !rhs.m_state) ? (!m_state && !rhs.m_state)
                                      : (reinterpret_cast<key&>(m_state->key_state) == 
                                         reinterpret_cast<key&>(rhs.m_state->key_state));
}

bool key_iterator::operator!=(const key_iterator& rhs) const noexcept { return !(*this == rhs); }

key_iterator::reference key_iterator::operator*() const
{
    assert(*this != key_iterator());
    return reinterpret_cast<const key&>(m_state->key_state);
}

key_iterator::pointer key_iterator::operator->() const
{
    assert(*this != key_iterator());
    return &reinterpret_cast<const key&>(m_state->key_state);
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
    ec.clear();
    LSTATUS rc;
    assert(*this != key_iterator());
    BOOST_SCOPE_EXIT_ALL(&, idx = m_state->key_idx) { if (ec || idx == m_state->key_idx) m_state.reset(); };

    do {
        DWORD buffer_size;
        (rc = RegQueryInfoKey(m_state->hkey, nullptr, nullptr, nullptr, nullptr, 
                              &buffer_size, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr)) ||
        (m_state->key_state.m_name.resize(m_state->subkey_pos + ++buffer_size), 0)                 ||
        (rc = RegEnumKeyEx(m_state->hkey, m_state->key_idx + 1, &m_state->key_state.m_name[0] + 
                           m_state->subkey_pos, &buffer_size, nullptr, nullptr, nullptr, nullptr)) ||
        (m_state->key_state.m_name.resize(m_state->subkey_pos + buffer_size), 0);
            
        if (rc != ERROR_SUCCESS && rc != ERROR_MORE_DATA) {
            ec = (rc == ERROR_NO_MORE_ITEMS) ? std::error_code{} : std::error_code(rc, std::system_category());
            return *this;
        }
    } while (rc == ERROR_MORE_DATA);
    
    ++m_state->key_idx; //the iterator is now pointing to the next element.
    return *this;
}

void key_iterator::swap(key_iterator& other) noexcept { m_state.swap(other.m_state); }


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
    ec.clear();
    assert(*this != recursive_key_iterator());
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) m_stack.clear(); };

    m_stack.emplace_back(*m_stack.back(), ec);
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

struct value_iterator::state
{
    shared_hkey                 hkey;
    uint32_t                    value_idx;
    details::value_entry_state  entry_state;
};

value_iterator::value_iterator() noexcept = default;

value_iterator::value_iterator(const value_iterator& other)
    : m_state(other.m_state ? std::make_unique<state>(*other.m_state) : nullptr)
{ }

value_iterator::value_iterator(value_iterator&& other) noexcept = default;

value_iterator& value_iterator::operator=(const value_iterator& other)
{
    if (this != &other) {
        swap(value_iterator(other));
    }
    return *this;
}

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
    if (key.empty()) {
        return; //return end iterator
    }
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) m_state.reset(); };

    HKEY hkey{};
    LSTATUS rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 
                              0, KEY_QUERY_VALUE | static_cast<DWORD>(key.view()), &hkey);
    if (rc != ERROR_SUCCESS) {
        ec = (rc == ERROR_FILE_NOT_FOUND) ? std::error_code{} : std::error_code(rc, std::system_category());
        return; //return end iterator
    }

    //NOTE: hkey will not leak, its ownership is thranfered before any other operation.
    m_state = std::make_unique<state>(state{
        hkey,
        uint32_t(-1),
        details::value_entry_state { key }
    });
    increment(ec);
}

value_iterator::~value_iterator() = default;

value_iterator& value_iterator::operator=(value_iterator&& other) noexcept = default;

bool value_iterator::operator==(const value_iterator& rhs) const noexcept
{
    return (!m_state || !rhs.m_state) ? (!m_state && !rhs.m_state)
                                      : (reinterpret_cast<value_entry&>(m_state->entry_state) ==
                                         reinterpret_cast<value_entry&>(rhs.m_state->entry_state));
}

bool value_iterator::operator!=(const value_iterator& rhs) const noexcept { return !(*this == rhs); }

value_iterator::reference value_iterator::operator*() const
{
    assert(*this != value_iterator());
    return reinterpret_cast<const value_entry&>(m_state->entry_state);
}

value_iterator::pointer value_iterator::operator->() const
{
    assert(*this != value_iterator());
    return &reinterpret_cast<const value_entry&>(m_state->entry_state);
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
    ec.clear();
    LSTATUS rc;
    assert(*this != value_iterator());
    BOOST_SCOPE_EXIT_ALL(&, idx = m_state->value_idx) { if (ec || idx == m_state->value_idx) m_state.reset(); };

    do {
        DWORD name_buf_sz;
        auto& entry = m_state->entry_state;
        (rc = RegQueryInfoKey(m_state->hkey, nullptr, nullptr, nullptr, nullptr, nullptr,
                              nullptr, nullptr, &name_buf_sz, nullptr, nullptr, nullptr))             ||
        (entry.m_value_name.resize(++name_buf_sz), 0)                                                 ||
        (rc = RegEnumValue(m_state->hkey, m_state->value_idx + 1, 
                           &entry.m_value_name[0], &name_buf_sz, nullptr, nullptr, nullptr, nullptr)) ||
        (entry.m_value_name.resize(name_buf_sz), 0);
            
        if (rc != ERROR_SUCCESS && rc != ERROR_MORE_DATA) {
            ec = (rc == ERROR_NO_MORE_ITEMS) ? std::error_code{} : std::error_code(rc, std::system_category());
            return *this;
        }
    } while (rc == ERROR_MORE_DATA);

    ++m_state->value_idx; //the iterator is now pointing to the next element.
    return *this;
}

void value_iterator::swap(value_iterator& other) noexcept { m_state.swap(other.m_state); }


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

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
    if (key.empty()) {
        return false;
    }

    HKEY hkey{};
    BOOST_SCOPE_EXIT_ALL(&) { unique_hkey(hkey); };
    LSTATUS rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 
                              0, KEY_QUERY_VALUE | static_cast<DWORD>(key.view()), &hkey);
    if (rc != ERROR_SUCCESS && rc != ERROR_FILE_NOT_FOUND) {
        ec = std::error_code(rc, std::system_category());
        return false;
    }
    return rc != ERROR_FILE_NOT_FOUND;
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
    if (key.empty()) {
        return false;
    }

    LSTATUS rc;
    HKEY hkey{};
    BOOST_SCOPE_EXIT_ALL(&) { unique_hkey(hkey); };
    (rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 
                       0, KEY_QUERY_VALUE | static_cast<DWORD>(key.view()), &hkey)) ||
    (rc = RegQueryValueEx(hkey, value_name.data(), nullptr, nullptr, nullptr, nullptr));
    
    if (rc != ERROR_SUCCESS && rc != ERROR_FILE_NOT_FOUND) {
        ec = std::error_code(rc, std::system_category());
        return false;
    }
    return rc != ERROR_FILE_NOT_FOUND;
}

key_time_type last_write_time(const key& key)
{
    std::error_code ec;
    decltype(auto) res = last_write_time(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

key_time_type last_write_time(const key& key, std::error_code& ec)
{
    LSTATUS rc;
    HKEY hkey{};
    FILETIME time;
    BOOST_SCOPE_EXIT_ALL(&) { unique_hkey(hkey); };
    (rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 
                       0, KEY_QUERY_VALUE | static_cast<DWORD>(key.view()), &hkey)) ||
    (rc = RegQueryInfoKey(hkey, nullptr, nullptr, nullptr, nullptr, nullptr, 
                          nullptr, nullptr, nullptr, nullptr, nullptr, &time));
    
    if (rc != ERROR_SUCCESS) {
        ec = std::error_code(rc, std::system_category());
        return key_time_type::min();
    }
    return key_time_type::clock::from_time_t(file_time_to_time_t(time));
}

value read_value(const key& key, string_view_type name)
{
    std::error_code ec;
    decltype(auto) res = read_value(key, name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, name);
    return res;
}

value read_value(const key& key, string_view_type name, std::error_code& ec)
{
    ec.clear();
    if (key.empty()) {
        ec = std::error_code(ERROR_FILE_NOT_FOUND, std::system_category());
        return value{};
    }

    HKEY hkey{};
    BOOST_SCOPE_EXIT_ALL(&) { unique_hkey(hkey); };
    LSTATUS rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 
                              0, KEY_QUERY_VALUE | static_cast<DWORD>(key.view()), &hkey);
    if (rc != ERROR_SUCCESS) {
        ec = std::error_code(rc, std::system_category());
        return value{};
    }
    
    details::value_state state;
    do {
        BYTE dummy;
        DWORD size = static_cast<DWORD>(state.m_data.size());
        rc = RegQueryValueEx(hkey, name.data(), nullptr, 
                             reinterpret_cast<DWORD*>(&state.m_type), size ? state.m_data.data() : &dummy, &size);
        state.m_data.resize(size);
    } while (rc == ERROR_MORE_DATA);
        
    if (rc != ERROR_SUCCESS) {
        ec = std::error_code(rc, std::system_category());
        return value{};
    }
    
    return reinterpret_cast<value&&>(state);
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
    if (key.empty()) {
        ec = std::error_code(ERROR_FILE_NOT_FOUND, std::system_category());
        return false;
    }

    if (!key.has_subkey()) {
        return false; //predefined keys are always there
    }

    HKEY hkey{};
    DWORD disposition;
    BOOST_SCOPE_EXIT_ALL(&) { unique_hkey(hkey); };
    LSTATUS rc = RegCreateKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 0, nullptr,
                                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | static_cast<DWORD>(key.view()), 
                                nullptr, &hkey, &disposition);
    
    if (rc != ERROR_SUCCESS) {
        ec = std::error_code(rc, std::system_category());
        return false;
    }
    return disposition == REG_CREATED_NEW_KEY;
}

bool create_keys(const key& key)
{
    std::error_code ec;
    decltype(auto) res = create_keys(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

bool create_keys(const key& key, std::error_code& ec)
{
    ec.clear();
    if (!key.empty() && key.has_parent_key()) {
        create_keys(key.parent_key(), ec);
    }
    return ec ? false : create_key(key, ec);
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
    if (key.empty()) {
        ec = std::error_code(ERROR_FILE_NOT_FOUND, std::system_category());
        return;
    }

    LSTATUS rc;
    HKEY hkey{};
    BOOST_SCOPE_EXIT_ALL(&) { unique_hkey(hkey); };
    (rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 
                       0, KEY_SET_VALUE | static_cast<DWORD>(key.view()), &hkey))     ||
    (rc = RegSetValueEx(hkey, value_name.data(), 0, static_cast<DWORD>(value.type()), 
                        value.data().data(), static_cast<DWORD>(value.data().size())));
    
    if (rc != ERROR_SUCCESS) {
        ec = std::error_code(rc, std::system_category());
    }
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
    ec.clear();
    if (key.empty()) {
        return false;
    }

    LSTATUS rc;
    if (!RegDeleteKeyEx_) {
        rc = RegDeleteKey(reinterpret_cast<HKEY>(key.root()), key.subkey().data());
    } else {
        rc = RegDeleteKeyEx_(reinterpret_cast<HKEY>(key.root()), 
                             key.subkey().data(), static_cast<DWORD>(key.view()), 0);
    }

    if (rc != ERROR_SUCCESS && rc != ERROR_FILE_NOT_FOUND) {
        ec = std::error_code(rc, std::system_category());
        return false;
    }
    return rc != ERROR_FILE_NOT_FOUND;
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
    if (key.empty()) {
        return false;
    }

    LSTATUS rc;
    HKEY hkey{};
    BOOST_SCOPE_EXIT_ALL(&) { unique_hkey(hkey); };
    (rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key.root()), key.subkey().data(), 
                       0, KEY_SET_VALUE | static_cast<DWORD>(key.view()), &hkey)) ||
    (rc = RegDeleteValue(hkey, value_name.data()));
    
    if (rc != ERROR_SUCCESS && rc != ERROR_FILE_NOT_FOUND) {
        ec = std::error_code(rc, std::system_category());
        return false;
    }
    return rc != ERROR_FILE_NOT_FOUND;
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
    ec.clear();
    if (key.empty()) {
        return 0;
    }

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
    static const auto nt_key_name = [](HKEY hkey) -> std::wstring
    {
        static constexpr int   KEY_NAME_INFORMATION =    3;
        static constexpr DWORD STATUS_BUFFER_TOO_SMALL = 0xC0000023L;

        DWORD rc, size = 0;
        std::vector<uint8_t> buffer;
        do {
            rc = NtQueryKey_(hkey, KEY_NAME_INFORMATION, buffer.data(), size, &size);
            if (rc == STATUS_BUFFER_TOO_SMALL) buffer.resize(size += sizeof(wchar_t) * 2);
        } while (rc == STATUS_BUFFER_TOO_SMALL);
        return rc ? std::wstring{} : std::wstring(reinterpret_cast<const wchar_t*>(buffer.data()) + 2);

        //TODO: this implementation assumes that no errors will occure in NtQueryKey
    };

    ec.clear();
    if (key1.empty() || key2.empty()) {
        ec = std::error_code(ERROR_FILE_NOT_FOUND, std::system_category());
        return false;
    }

    LSTATUS rc;
    HKEY hkey1{}, hkey2{};
    BOOST_SCOPE_EXIT_ALL(&) { unique_hkey(hkey1); unique_hkey(hkey2); };
    (rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key1.root()), key1.subkey().data(), 
                       0, KEY_QUERY_VALUE | static_cast<DWORD>(key1.view()), &hkey1)) ||
    (rc = RegOpenKeyEx(reinterpret_cast<HKEY>(key2.root()), key2.subkey().data(), 
                       0, KEY_QUERY_VALUE | static_cast<DWORD>(key2.view()), &hkey2));
    
    if (rc != ERROR_SUCCESS) {
        ec = std::error_code(rc, std::system_category());
        return false;
    }
    return nt_key_name(hkey1) == nt_key_name(hkey2);
}

}  // namespace registry