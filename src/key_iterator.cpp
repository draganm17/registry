#include <cassert>
#include <Windows.h>

#include <boost/scope_exit.hpp>

#include <registry/details/utils.impl.h>
#include <registry/exception.h>
#include <registry/key_iterator.h>
#include <registry/operations.h>


namespace registry {

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

key_info key_entry::info(key_info_mask mask, std::error_code& ec) const
{
    std::error_code ec2;
    const auto handle = m_key_handle.lock();
    auto result = handle.valid() ? handle.info(mask, ec2) : registry::info(m_key, mask, ec2);

    if (!ec2) RETURN_RESULT(ec, result);
    details::set_or_throw(&ec, ec2, __FUNCTION__, m_key);
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
//                              class key_iterator                                    //
//------------------------------------------------------------------------------------//

struct key_iterator::state
{
    uint32_t                              idx;
    key_handle                            hkey;
    key_entry                             entry;
    std::vector<string_type::value_type>  buffer;
};

key_iterator::key_iterator(const key& key, std::error_code& ec)
{
    std::error_code ec2;
    const auto handle = open(key, access_rights::enumerate_sub_keys, ec2);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, VOID);

    key_iterator tmp;
    if (!ec2 && (tmp = key_iterator(handle, ec2), !ec2)) {
        swap(tmp);
        RETURN_RESULT(ec, VOID);
    }
    details::set_or_throw(&ec, ec2, __FUNCTION__, key);
}

key_iterator::key_iterator(const key_handle& handle, std::error_code& ec)
    : m_state(std::make_shared<state>(state{ uint32_t(-1), handle, key_entry(handle) }))
{
    std::error_code ec2;
    key_info info = handle.info(key_info_mask::read_max_subkey_size, ec2);

    if (!ec2) {
        m_state->buffer.resize(++info.max_subkey_size, TEXT('_'));
        m_state->entry.m_key.append({ m_state->buffer.data(), m_state->buffer.size() });
        if (increment(ec2), !ec2) RETURN_RESULT(ec, VOID);
    }
    m_state.reset();
    details::set_or_throw(&ec, ec2, __FUNCTION__, handle.key());
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
        DWORD buffer_size = m_state->buffer.size();
        HKEY hkey = reinterpret_cast<HKEY>(m_state->hkey.native_handle());
        LSTATUS rc = RegEnumKeyEx(hkey, ++m_state->idx, m_state->buffer.data(), 
                                  &buffer_size, nullptr, nullptr, nullptr, nullptr);

        if (!(ec = std::error_code(rc, std::system_category()))) {
            m_state->entry.m_key.replace_leaf({ m_state->buffer.data(), buffer_size });
        }
    } while (ec.value() == ERROR_MORE_DATA);

    return *this;
}

void key_iterator::swap(key_iterator& other) noexcept { m_state.swap(other.m_state); }


//------------------------------------------------------------------------------------//
//                         class recursive_key_iterator                               //
//------------------------------------------------------------------------------------//

recursive_key_iterator::recursive_key_iterator(const key& key, std::error_code& ec)
{
    std::error_code ec2;
    m_stack.emplace_back(key, ec2);

    if (!ec2) {
        if (m_stack.back() == key_iterator()) m_stack.clear();
        RETURN_RESULT(ec, VOID);
    }
    m_stack.clear();
    if (!ec2) details::set_or_throw(&ec, ec2, __FUNCTION__, key);
}

recursive_key_iterator::recursive_key_iterator(const key_handle& handle, std::error_code& ec)
{
    std::error_code ec2;
    m_stack.emplace_back(handle, ec2);
    if (!ec2) RETURN_RESULT(ec, VOID);

    m_stack.clear();
    details::set_or_throw(&ec, ec2, __FUNCTION__, handle.key());
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