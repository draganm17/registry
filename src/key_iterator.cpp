#include <cassert>
#include <Windows.h>

#include <boost/scope_exit.hpp>

#include <registry/details/utils.impl.h>
#include <registry/key_iterator.h>
#include <registry/operations.h>


namespace 
{
    using namespace registry;

    key_info_mask get_mask(const key_info& info) noexcept
    {
        constexpr key_info empty_v{};
        key_info_mask mask = key_info_mask::none;

        if (info.subkeys             != empty_v.subkeys)             mask |= key_info_mask::read_subkeys;
        if (info.values              != empty_v.values)              mask |= key_info_mask::read_values;
        if (info.max_subkey_size     != empty_v.max_subkey_size)     mask |= key_info_mask::read_max_subkey_size;
        if (info.max_value_name_size != empty_v.max_value_name_size) mask |= key_info_mask::read_max_value_name_size;
        if (info.max_value_data_size != empty_v.max_value_data_size) mask |= key_info_mask::read_max_value_data_size;
        if (info.last_write_time     != empty_v.last_write_time)     mask |= key_info_mask::read_last_write_time;

        return mask;
    }

    void update(key_info& dst, key_info& src) noexcept
    {
        constexpr key_info empty_v{};

        if (src.subkeys             != empty_v.subkeys)             dst.subkeys = src.subkeys;
        if (src.values              != empty_v.values)              dst.values = src.values;
        if (src.max_subkey_size     != empty_v.max_subkey_size)     dst.max_subkey_size = src.max_subkey_size;
        if (src.max_value_name_size != empty_v.max_value_name_size) dst.max_value_name_size = src.max_value_name_size;
        if (src.max_value_data_size != empty_v.max_value_data_size) dst.max_value_data_size = src.max_value_data_size;
        if (src.last_write_time     != empty_v.last_write_time)     dst.last_write_time = src.last_write_time;
    }
}

namespace registry {

//------------------------------------------------------------------------------------//
//                               class key_entry                                      //
//------------------------------------------------------------------------------------//

key_entry::key_entry(const registry::key& key)
    : m_key(key)
{
    refresh();
}

key_entry::key_entry(const registry::key& key, std::error_code& ec)
    : m_key(key)
{
    ec.clear();
    refresh(ec);
    if (ec) swap(key_entry());
}

key_entry::key_entry(const key_handle& handle)
    : m_key(handle.key())
    , m_key_handle(handle)
{
    refresh();
}

key_entry::key_entry(const key_handle& handle, std::error_code& ec)
    : m_key(handle.key())
    , m_key_handle(handle)
{
    ec.clear();
    refresh(ec);
    if (ec) swap(key_entry());
}

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
    auto info = m_key_info;
    if ((mask = mask & ~get_mask(m_key_info)) != key_info_mask::none) {
        auto handle = m_key_handle.lock();
        update(info, handle.valid() ? handle.info(mask, ec) : registry::info(m_key, mask, ec));
    }
    return !ec ? info : key_info{};
}

key_entry& key_entry::assign(const registry::key& key)
{ 
    m_key = key;
    m_key_handle.swap(weak_key_handle());
    return *this;
}

key_entry& key_entry::assign(const registry::key& key, std::error_code& ec)
{
    // TODO: ...
    return *this;
}

key_entry& key_entry::assign(const key_handle& handle)
{
    m_key.swap(registry::key());
    m_key_handle = handle;
    return *this;
}

key_entry& key_entry::assign(const key_handle& handle, std::error_code& ec)
{
    // TODO: ...
    return *this;
}

void key_entry::refresh(key_info_mask mask)
{
    std::error_code ec;
    refresh(mask, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, m_key);
}

void key_entry::refresh(key_info_mask mask, std::error_code& ec)
{
    ec.clear();
    auto handle = m_key_handle.lock();
    update(m_key_info, handle.valid() ? handle.info(mask, ec) : registry::info(m_key, mask, ec));
}

void key_entry::swap(key_entry& other) noexcept
{
    // TODO: ...
    //m_key.swap(other.m_key);
}


//------------------------------------------------------------------------------------//
//                              class key_iterator                                    //
//------------------------------------------------------------------------------------//

struct key_iterator::state
{
    uint32_t                              idx;
    key_handle                            hkey;
    key_entry                             entry;
    std::vector<string_type::value_type>  buffer;
};

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
    auto handle = open(key, access_rights::enumerate_sub_keys, ec);

    if (!ec) swap(key_iterator(handle, ec)); else
    if (ec.value() == ERROR_FILE_NOT_FOUND) ec.clear();
}

key_iterator::key_iterator(const key_handle& handle)
{
    std::error_code ec;
    auto tmp = key_iterator(handle, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, handle.key());
    swap(tmp);
}

key_iterator::key_iterator(const key_handle& handle, std::error_code& ec)
    : m_state(std::make_shared<state>(state{ uint32_t(-1), handle, key_entry(handle) }))
{
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) m_state.reset(); };
    key_info info = handle.info(key_info_mask::read_max_subkey_size, ec);

    if (!ec) {
        m_state->buffer.resize(++info.max_subkey_size, TEXT('_'));
        m_state->entry.m_key.append({ m_state->buffer.data(), m_state->buffer.size() });
        increment(ec);
    }
}

bool key_iterator::operator==(const key_iterator& rhs) const noexcept { return m_state == rhs.m_state; }

bool key_iterator::operator!=(const key_iterator& rhs) const noexcept { return !(*this == rhs); }

key_iterator::reference key_iterator::operator*() const
{
    assert(*this != key_iterator());
    return m_state->entry;
}

key_iterator::pointer key_iterator::operator->() const
{
    assert(*this != key_iterator());
    return &m_state->entry;
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
        if (ec) m_state.reset();
        if (ec.value() == ERROR_NO_MORE_ITEMS) ec.clear();
    };

    // NOTE: Subkeys which names size exceed the size of the pre-allocated buffer are ignored.
    //       Such values may only appear in the enumerated sequence if they were added to the registry key after the
    //       iterator was constructed. Therefore this behaviour is consistent with what the class documentation states.

    do {
        FILETIME time;
        DWORD buffer_size = m_state->buffer.size();
        HKEY hkey = reinterpret_cast<HKEY>(m_state->hkey.native_handle());
        LSTATUS rc = RegEnumKeyEx(hkey, ++m_state->idx, m_state->buffer.data(), 
                                  &buffer_size, nullptr, nullptr, nullptr, &time);

        if (!(ec = std::error_code(rc, std::system_category()))) {
            auto& entry = m_state->entry;
            entry.m_key.replace_leaf({ m_state->buffer.data(), buffer_size });
            entry.m_key_info.last_write_time = key_time_type::clock::from_time_t(details::file_time_to_time_t(time));
        }
    } while (ec.value() == ERROR_MORE_DATA);

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
    m_stack.emplace_back(key, ec);
    if (ec || m_stack.back() == key_iterator()) m_stack.clear();
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
    m_stack.emplace_back(handle, ec);
    if (ec) m_stack.clear();
}

bool recursive_key_iterator::operator==(const recursive_key_iterator& rhs) const noexcept
{
    return 0;
    // TODO: ...
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

}  // namespace registry