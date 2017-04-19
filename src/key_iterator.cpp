#include <array>
#include <cassert>
#include <cstdint>
#include <Windows.h>

#include <registry/details/utils.impl.h>
#include <registry/exception.h>
#include <registry/key_iterator.h>
#include <registry/operations.h>


namespace {

    constexpr size_t MAX_KEY_SIZE = 260;

}


namespace registry {

//------------------------------------------------------------------------------------//
//                               class key_entry                                      //
//------------------------------------------------------------------------------------//

key_entry::key_entry(const key_path& path)
    : m_path(path)
{ }

const key_path& key_entry::path() const noexcept { return m_path; }

key_info key_entry::info(key_info_mask mask, std::error_code& ec) const
{
    std::error_code ec2;
    const auto handle = m_key_handle.lock();
    auto result = handle ? handle->info(mask, ec2) : registry::info(m_path, mask, ec2);

    if (!ec2) RETURN_RESULT(ec, result);
    details::set_or_throw(&ec, ec2, __FUNCTION__, m_path);
}

key_entry& key_entry::assign(const key_path& path)
{ 
    m_path = path;
    m_key_handle.swap(std::weak_ptr<key_handle>());
    return *this;
}

void key_entry::swap(key_entry& other) noexcept
{
    using std::swap;
    swap(m_path, other.m_path);
    swap(m_key_handle, other.m_key_handle);
}


//------------------------------------------------------------------------------------//
//                              class key_iterator                                    //
//------------------------------------------------------------------------------------//

struct key_iterator::state
{
    uint32_t                                           idx;
    key_handle                                         hkey;
    key_entry                                          entry;
    std::array<string_type::value_type, MAX_KEY_SIZE>  buffer;
};

key_iterator::key_iterator(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    key_handle handle(path, access_rights::enumerate_sub_keys | access_rights::query_value, ec2);

    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, VOID);
    if (!ec2 && (swap(key_iterator(std::move(handle), ec2)), !ec2)) RETURN_RESULT(ec, VOID);

    details::set_or_throw(&ec, ec2, __FUNCTION__, path);
}

key_iterator::key_iterator(key_handle handle, std::error_code& ec)
    : m_state(std::make_shared<state>(state{ uint32_t(-1), std::move(handle) }))
{
    m_state->entry.m_path = m_state->hkey.path();
    m_state->entry.m_key_handle = std::shared_ptr<key_handle>(m_state, &m_state->hkey);

    std::error_code ec2;
    m_state->entry.m_path.append(TEXT("PLACEHOLDER"));
    if (increment(ec2), !ec2) RETURN_RESULT(ec, VOID);

    key_iterator tmp(std::move(*this));
    details::set_or_throw(&ec, ec2, __FUNCTION__, tmp.m_state->hkey.path());
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

    try {
        LSTATUS rc;
        DWORD buffer_size = m_state->buffer.size();
        rc = RegEnumKeyEx(reinterpret_cast<HKEY>(m_state->hkey.native_handle()), ++m_state->idx,
                          m_state->buffer.data(), &buffer_size, nullptr, nullptr, nullptr, nullptr);

        if (rc == ERROR_SUCCESS) {
            m_state->entry.m_path.replace_leaf({ m_state->buffer.data(), buffer_size });
        } else if (rc == ERROR_NO_MORE_ITEMS) {
            key_iterator tmp(std::move(*this)); // *this becomes the end iterator
        } else {
            key_iterator tmp(std::move(*this)); // *this becomes the end iterator
            return details::set_or_throw(&ec, std::error_code(rc, std::system_category()), __FUNCTION__), *this;
        }

        RETURN_RESULT(ec, *this);
    } catch(...) {
        key_iterator(std::move(*this)); // *this becomes the end iterator (if not already)
        throw;
    }
}

void key_iterator::swap(key_iterator& other) noexcept { m_state.swap(other.m_state); }


//------------------------------------------------------------------------------------//
//                         class recursive_key_iterator                               //
//------------------------------------------------------------------------------------//

recursive_key_iterator::recursive_key_iterator(const key_path& path, key_options options, std::error_code& ec)
    : m_options(options)
{
    std::error_code ec2;
    key_handle handle(path, access_rights::enumerate_sub_keys | access_rights::query_value, ec2);
    const bool skip_permission_denied = (options & key_options::skip_permission_denied) != key_options::none;
    
    if (ec2.value() == ERROR_FILE_NOT_FOUND || 
        (ec2.value() == ERROR_ACCESS_DENIED && skip_permission_denied)) RETURN_RESULT(ec, VOID);

    if (!ec2 && (swap(recursive_key_iterator(std::move(handle), options, ec2)), !ec2)) RETURN_RESULT(ec, VOID);

    details::set_or_throw(&ec, ec2, __FUNCTION__, path);
}

recursive_key_iterator::recursive_key_iterator(key_handle handle, key_options options, std::error_code& ec)
    : m_options(options)
{
    std::error_code ec2;
    if (m_stack.emplace_back(std::move(handle), ec2), !ec2) RETURN_RESULT(ec, VOID);

    m_stack.clear();
    details::set_or_throw(&ec, ec2, __FUNCTION__, handle.path());
}

bool recursive_key_iterator::operator==(const recursive_key_iterator& rhs) const noexcept
{
    return m_stack == rhs.m_stack; // TODO: is that right ???
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

key_options recursive_key_iterator::options() const
{
    assert(*this != recursive_key_iterator());
    return m_options;
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
    // TODO: guarantee that '*this' is set ot the end iterator on error

    assert(*this != recursive_key_iterator());

    ec.clear();
    m_stack.emplace_back(m_stack.back()->path(), ec);
    const bool skip_pd = (m_options & key_options::skip_permission_denied) != key_options::none;
    
    if (ec.value() == ERROR_ACCESS_DENIED && skip_pd) ec.clear();
    while (!ec && !m_stack.empty() && m_stack.back() == key_iterator())
    {
        m_stack.pop_back();
        if (m_stack.size()) m_stack.back().increment(ec);
    }
    return ec ? (m_stack.clear(), *this) : *this;
}

void recursive_key_iterator::pop(std::error_code& ec)
{
    // TODO: guarantee that '*this' is set ot the end iterator on error

    assert(*this != recursive_key_iterator());

    std::error_code ec2;
    do {
        m_stack.pop_back();
        if (m_stack.size()) m_stack.back().increment(ec2);
    } while (!ec2 && !m_stack.empty() && m_stack.back() == key_iterator());
    
    if (!ec2) RETURN_RESULT(ec, VOID);
    details::set_or_throw(&ec, ec2, __FUNCTION__);
}

void recursive_key_iterator::swap(recursive_key_iterator& other) noexcept
{
    using std::swap;
    swap(m_stack, other.m_stack);
    swap(m_options, other.m_options);
}

}  // namespace registry