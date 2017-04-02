#include <cassert>
#include <Windows.h>

#include <boost/scope_exit.hpp>

#include <registry/operations.h>
#include <registry/value_iterator.h>


namespace registry {

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
//                             class value_iterator                                   //
//------------------------------------------------------------------------------------//

struct value_iterator::state
{
    uint32_t                              idx;
    key_handle                            hkey;
    value_entry                           entry;
    std::vector<string_type::value_type>  buffer;

};

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
    auto handle = open(key, access_rights::query_value, ec);

    if (!ec) swap(value_iterator(handle, ec)); else
    if (ec.value() == ERROR_FILE_NOT_FOUND) ec.clear();
}

value_iterator::value_iterator(const key_handle& handle)
{
    std::error_code ec;
    auto tmp = value_iterator(handle, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, handle.key());
    swap(tmp);
}

value_iterator::value_iterator(const key_handle& handle, std::error_code& ec)
    : m_state(std::make_shared<state>(state{ uint32_t(-1), handle, value_entry(handle, string_type()) }))
{
    ec.clear();
    BOOST_SCOPE_EXIT_ALL(&) { if (ec) m_state.reset(); };
    key_info info = handle.info(key_info_mask::read_max_value_name_size, ec);

    if (!ec) {
        m_state->buffer.resize(++info.max_value_name_size);
        m_state->entry.m_value_name.reserve(info.max_value_name_size);
        increment(ec);
    }
}

bool value_iterator::operator==(const value_iterator& rhs) const noexcept { return m_state == rhs.m_state; }

bool value_iterator::operator!=(const value_iterator& rhs) const noexcept { return !(*this == rhs); }

value_iterator::reference value_iterator::operator*() const
{
    assert(*this != value_iterator());
    return m_state->entry;
}

value_iterator::pointer value_iterator::operator->() const
{
    assert(*this != value_iterator());
    return &m_state->entry;
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
        if (ec) m_state.reset();
        if (ec.value() == ERROR_NO_MORE_ITEMS) ec.clear();
    };

    // NOTE: Values which names size exceed the size of the pre-allocated buffer are ignored.
    //       Such values may only appear in the enumerated sequence if they were added to the registry key after the
    //       iterator was constructed. Therefore this behaviour is consistent with what the class documentation states.

    do {
        DWORD buffer_size = m_state->buffer.size();
        HKEY hkey = reinterpret_cast<HKEY>(m_state->hkey.native_handle());
        LSTATUS rc = RegEnumValue(hkey, ++m_state->idx, m_state->buffer.data(), 
                                  &buffer_size, nullptr, nullptr, nullptr, nullptr);

        if (!(ec = std::error_code(rc, std::system_category()))) {
            m_state->entry.m_value_name.assign(m_state->buffer.data(), buffer_size);
        }
    } while (ec.value() == ERROR_MORE_DATA);

    return *this;
}

void value_iterator::swap(value_iterator& other) noexcept { m_state.swap(other.m_state); }

}  // namespace registry