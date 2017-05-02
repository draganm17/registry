#include <cassert>
#include <Windows.h>

#include <registry/details/utils.impl.h>
#include <registry/exception.h>
#include <registry/key.h>
#include <registry/operations.h>
#include <registry/value_iterator.h>


namespace registry {

//------------------------------------------------------------------------------------//
//                              class value_entry                                     //
//------------------------------------------------------------------------------------//

value_entry::value_entry(const key_path& path, string_view_type value_name)
    : m_path(path)
    , m_value_name(value_name)
{ }

const key_path& value_entry::path() const noexcept { return m_path; }

const string_type& value_entry::value_name() const noexcept { return m_value_name; }

value value_entry::value(std::error_code& ec) const
{
    std::error_code ec2;
    const auto key_ptr = m_key_weak_ptr.lock();
    auto result = key_ptr ? key_ptr->read_value(m_value_name, ec2) : read_value(m_path, m_value_name, ec2);

    if (!ec2) RETURN_RESULT(ec, result);
    details::set_or_throw(&ec, ec2, __FUNCTION__, m_path, key_path(), m_value_name);
}

value_entry& value_entry::assign(const key_path& path, string_view_type value_name)
{
    m_path = path;
    m_value_name.assign(value_name.data(), value_name.size());
    m_key_weak_ptr.swap(std::weak_ptr<key>());
    return *this;
}

void value_entry::swap(value_entry& other) noexcept
{
    using std::swap;
    swap(m_path, other.m_path);
    swap(m_value_name, other.m_value_name);
    swap(m_key_weak_ptr, other.m_key_weak_ptr);
}


//------------------------------------------------------------------------------------//
//                             class value_iterator                                   //
//------------------------------------------------------------------------------------//

struct value_iterator::state
{
    uint32_t                              idx;
    key                                   key;
    value_entry                           val;
    std::vector<string_type::value_type>  buf;  // TODO: write data directly to val.m_key_name ???

};

value_iterator::value_iterator(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    key k(open_only_tag{}, path, access_rights::query_value, ec2);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, VOID);

    key_info info;
    m_state = std::make_shared<state>(state{ uint32_t(-1), 
                                             std::move(k),
                                             value_entry(path, string_type()) });
    if (!ec2 && (info = m_state->key.info(key_info_mask::read_max_value_name_size, ec2), !ec2))
    {
        m_state->buf.resize(++info.max_value_name_size);
        m_state->val.m_value_name.reserve(info.max_value_name_size);
        m_state->val.m_key_weak_ptr = std::shared_ptr<key>(m_state, &m_state->key);

        if (increment(ec2), !ec2) RETURN_RESULT(ec, VOID);
    }

    swap(value_iterator());
    details::set_or_throw(&ec, ec2, __FUNCTION__, path);
}

//value_iterator::value_iterator(const key& key, std::error_code& ec) : value_iterator(key.path(), ec) { }

bool value_iterator::operator==(const value_iterator& rhs) const noexcept { return m_state == rhs.m_state; }

bool value_iterator::operator!=(const value_iterator& rhs) const noexcept { return !(*this == rhs); }

value_iterator::reference value_iterator::operator*() const
{
    assert(*this != value_iterator());
    return m_state->val;
}

value_iterator::pointer value_iterator::operator->() const
{
    assert(*this != value_iterator());
    return &m_state->val;
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
    LSTATUS rc;
    assert(*this != value_iterator());

    // NOTE: Values which names size exceed the size of the pre-allocated buffer are ignored.
    //       Such values may only appear in the enumerated sequence if they were added to the registry key after the
    //       iterator was constructed. Therefore this behaviour is consistent with what the class documentation states.

    try {
        do {
            DWORD buffer_size = m_state->buf.size();
            rc = RegEnumValue(reinterpret_cast<HKEY>(m_state->key.native_handle()), ++m_state->idx,
                              m_state->buf.data(), &buffer_size, nullptr, nullptr, nullptr, nullptr);

            if (rc == ERROR_SUCCESS) {
                m_state->val.m_value_name.assign(m_state->buf.data(), buffer_size);
            } else if (rc == ERROR_NO_MORE_ITEMS) {
                value_iterator tmp(std::move(*this)); // *this becomes the end iterator
            } else if (rc != ERROR_MORE_DATA) {
                value_iterator tmp(std::move(*this)); // *this becomes the end iterator
                return details::set_or_throw(&ec, std::error_code(rc, std::system_category()), __FUNCTION__), *this;
            }
        } while (rc == ERROR_MORE_DATA);

        RETURN_RESULT(ec, *this);
    } catch(...) {
        value_iterator(std::move(*this)); // *this becomes the end iterator (if not already)
        throw;
    }
}

void value_iterator::swap(value_iterator& other) noexcept { m_state.swap(other.m_state); }

}  // namespace registry