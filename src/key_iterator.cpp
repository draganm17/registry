#include <array>
#include <cassert>
#include <cstdint>
#include <Windows.h>

#include <registry/details/utils.impl.h>
#include <registry/exception.h>
#include <registry/key.h>
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
    const auto key_ptr = m_key_weak_ptr.lock();
    auto result = key_ptr ? key_ptr->info(mask, ec2) : registry::info(m_path, mask, ec2);

    if (!ec2) RETURN_RESULT(ec, result);
    details::set_or_throw(&ec, ec2, __FUNCTION__, m_path);
}

key_entry& key_entry::assign(const key_path& path)
{ 
    m_path = path;
    m_key_weak_ptr.swap(std::weak_ptr<key>());
    return *this;
}

void key_entry::swap(key_entry& other) noexcept
{
    using std::swap;
    swap(m_path, other.m_path);
    swap(m_key_weak_ptr, other.m_key_weak_ptr);
}


//------------------------------------------------------------------------------------//
//                              class key_iterator                                    //
//------------------------------------------------------------------------------------//

struct key_iterator::state
{
    uint32_t                                           idx;
    key                                                key;
    key_entry                                          val;
    std::array<string_type::value_type, MAX_KEY_SIZE>  buf;
};

key_iterator::key_iterator(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    key k(open_only_tag{}, path, access_rights::enumerate_sub_keys | access_rights::query_value, ec2);

    if (!ec2) {
        m_state = std::make_shared<state>(state{ uint32_t(-1),
                                                 std::move(k),
                                                 key_entry(key_path(path).append(TEXT("PLACEHOLDER"))) });

        m_state->val.m_key_weak_ptr = std::shared_ptr<key>(m_state, &m_state->key);
        
        if (increment(ec2), !ec2) RETURN_RESULT(ec, VOID);
    } else if (ec2.value() == ERROR_FILE_NOT_FOUND) {
        RETURN_RESULT(ec, VOID);
    }

    swap(key_iterator());
    details::set_or_throw(&ec, ec2, __FUNCTION__, path);
}

//key_iterator::key_iterator(const key& key, std::error_code& ec) : key_iterator(key.path(), ec) { }

bool key_iterator::operator==(const key_iterator& rhs) const noexcept { return m_state == rhs.m_state; }

bool key_iterator::operator!=(const key_iterator& rhs) const noexcept { return !(*this == rhs); }

key_iterator::reference key_iterator::operator*() const
{
    assert(*this != key_iterator());
    return m_state->val;
}

key_iterator::pointer key_iterator::operator->() const
{
    assert(*this != key_iterator());
    return &m_state->val;
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
        DWORD buffer_size = m_state->buf.size();
        rc = RegEnumKeyEx(reinterpret_cast<HKEY>(m_state->key.native_handle()), ++m_state->idx,
                          m_state->buf.data(), &buffer_size, nullptr, nullptr, nullptr, nullptr);

        if (rc == ERROR_SUCCESS) {
            m_state->val.m_path.replace_leaf_key({ m_state->buf.data(), buffer_size });
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

recursive_key_iterator::recursive_key_iterator(const key_path& path, std::error_code& ec)
    : recursive_key_iterator(path, key_options::none, ec) { }

recursive_key_iterator::recursive_key_iterator(const key_path& path, key_options options, std::error_code& ec)
    : m_options(options)
{
    std::error_code ec2;
    m_stack.emplace_back(path, ec2);
    if (!ec2 && m_stack.back() != key_iterator()) RETURN_RESULT(ec, VOID);
    
    swap(recursive_key_iterator());
    if (!ec2) RETURN_RESULT(ec, VOID);
    if ((ec2.value() == ERROR_ACCESS_DENIED) &&
        (options & key_options::skip_permission_denied) != key_options::none)
    {
        RETURN_RESULT(ec, VOID);
    }
    details::set_or_throw(&ec, ec2, __FUNCTION__, path);
}

//recursive_key_iterator::recursive_key_iterator(const key& key, std::error_code& ec)
//    : recursive_key_iterator(key, key_options::none, ec) { }

//recursive_key_iterator::recursive_key_iterator(const key& key, key_options options, std::error_code& ec)
//    : recursive_key_iterator(key, options, ec) { }

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