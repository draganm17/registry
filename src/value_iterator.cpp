#include <cassert>
#include <Windows.h>

#include <registry/details/utils.impl.h>
#include <registry/exception.h>
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

const key& value_entry::key() const noexcept { return m_key; }

const string_type& value_entry::value_name() const noexcept { return m_value_name; }

value value_entry::value(std::error_code& ec) const
{
    std::error_code ec2;
    const auto handle = m_key_handle.lock();
    auto result = handle ? handle->read_value(m_value_name, ec2) : read_value(m_key, m_value_name, ec2);

    if (!ec2) RETURN_RESULT(ec, result);
    details::set_or_throw(&ec, ec2, __FUNCTION__, m_key, registry::key(), m_value_name);
}

value_entry& value_entry::assign(const registry::key& key, string_view_type value_name)
{
    m_key = key;
    m_value_name.assign(value_name.data(), value_name.size());
    m_key_handle.swap(std::weak_ptr<key_handle>());
    return *this;
}

void value_entry::swap(value_entry& other) noexcept
{
    using std::swap;
    swap(m_key, other.m_key);
    swap(m_value_name, other.m_value_name);
    swap(m_key_handle, other.m_key_handle);
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

value_iterator::value_iterator(const key& key, std::error_code& ec)
{
    std::error_code ec2;
    key_handle handle(key, access_rights::query_value, ec2);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, VOID);
    if (!ec2 && (swap(value_iterator(std::move(handle), ec2)), !ec2)) RETURN_RESULT(ec, VOID);

    details::set_or_throw(&ec, ec2, __FUNCTION__, key);
}

value_iterator::value_iterator(key_handle handle, std::error_code& ec)
    : m_state(std::make_shared<state>(state{ uint32_t(-1), std::move(handle) }))
{
    m_state->entry.m_key = m_state->hkey.key();
    m_state->entry.m_key_handle = std::shared_ptr<registry::key_handle>(m_state, &m_state->hkey);

    std::error_code ec2;
    key_info info = m_state->hkey.info(key_info_mask::read_max_value_name_size, ec2);

    if (!ec2) {
        m_state->buffer.resize(++info.max_value_name_size);
        m_state->entry.m_value_name.reserve(info.max_value_name_size);
        if (increment(ec2), !ec2) RETURN_RESULT(ec, VOID);
    }

    key key = m_state->hkey.key();
    m_state.reset(); // *this becomes the end iterator
    details::set_or_throw(&ec, ec2, __FUNCTION__, std::move(key));
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
    // TODO: guarantee forward progress on error

    LSTATUS rc;
    assert(*this != value_iterator());

    // NOTE: Values which names size exceed the size of the pre-allocated buffer are ignored.
    //       Such values may only appear in the enumerated sequence if they were added to the registry key after the
    //       iterator was constructed. Therefore this behaviour is consistent with what the class documentation states.

    do {
        DWORD buffer_size = m_state->buffer.size();
        rc = RegEnumValue(reinterpret_cast<HKEY>(m_state->hkey.native_handle()), ++m_state->idx,
                          m_state->buffer.data(), &buffer_size, nullptr, nullptr, nullptr, nullptr);

        if (rc == ERROR_SUCCESS) {
            m_state->entry.m_value_name.assign(m_state->buffer.data(), buffer_size);
        } else if (rc == ERROR_NO_MORE_ITEMS) {
            m_state.reset(); // *this becomes the end iterator
        } else if (rc != ERROR_MORE_DATA) {
            m_state.reset(); // *this becomes the end iterator
            const std::error_code ec2(rc, std::system_category());
            return details::set_or_throw(&ec, ec2, __FUNCTION__), *this;
        }
    } while (rc == ERROR_MORE_DATA);

    RETURN_RESULT(ec, *this);
}

void value_iterator::swap(value_iterator& other) noexcept { m_state.swap(other.m_state); }

}  // namespace registry