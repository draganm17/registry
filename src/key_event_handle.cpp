#include <algorithm>
#include <Windows.h>

#include <registry/details/utils.impl.h>
#include <registry/key.h>
#include <registry/key_event_handle.h>
#include <registry/key_handle.h>


namespace {

using namespace registry;

void close_handle(key_event_handle::native_handle_type handle, std::error_code& ec) noexcept
{
    ec.clear();
    if (!(HANDLE)handle) return;
    const LSTATUS rc = CloseHandle((HANDLE)handle) ? ERROR_SUCCESS : GetLastError();
    if (rc != ERROR_SUCCESS) ec = std::error_code(rc, std::system_category());
}

}


namespace registry {

void key_event_handle::close_handle_t::operator()(void* handle) const noexcept
{ CloseHandle(reinterpret_cast<HANDLE>(handle)); }

key_event_status key_event_handle::wait_impl(
    const char* caller, std::chrono::milliseconds rel_time, std::error_code& ec) const
{
    assert(is_open());  // TODO: ???

    using rep_t = std::chrono::milliseconds::rep;
    const DWORD rc = WaitForSingleObject(reinterpret_cast<HANDLE>(m_event_handle.get()), 
                                         (DWORD)std::min(std::max(rel_time.count(), rep_t(0)), rep_t(INFINITE)));

    if (rc == WAIT_TIMEOUT) RETURN_RESULT(ec, key_event_status::timeout);
    if (rc == WAIT_OBJECT_0) RETURN_RESULT(ec, key_event_status::signalled);

    const std::error_code ec2(GetLastError(), std::system_category());
    return details::set_or_throw(&ec, ec2, caller, m_key_handle.key()), key_event_status::failed;
}

key_event_handle::key_event_handle(const key& key, key_event_filter filter, bool watch_subtree, std::error_code& ec)
{
    std::error_code ec2;
    key_handle handle(key, access_rights::notify, ec2);
    if (!ec2 && (swap(key_event_handle(std::move(handle), filter, watch_subtree, ec2)), !ec2)) RETURN_RESULT(ec, VOID);

    details::set_or_throw(&ec, ec2, __FUNCTION__, key);
}

key_event_handle::key_event_handle(key_handle handle, key_event_filter filter, bool watch_subtree, std::error_code& ec)
    : m_watch_subtree(watch_subtree)
    , m_filter(filter)
    , m_key_handle(std::move(handle))
{
    // TODO: check (and refactor ???)

    if (filter == key_event_filter::notify_none) {
        swap(key_event_handle());
        RETURN_RESULT(ec, VOID);
    }

    LRESULT rc{};
    HANDLE hevent;
    if ((hevent = CreateEvent(nullptr, TRUE, FALSE, nullptr)) &&
        !(rc = RegNotifyChangeKeyValue(reinterpret_cast<HKEY>(m_key_handle.native_handle()),
                                       static_cast<BOOL>(watch_subtree), static_cast<DWORD>(filter), hevent, TRUE)))
    {
        RETURN_RESULT(ec, VOID);
    }

    const std::error_code ec2(rc ? rc : GetLastError(), std::system_category());
    key key = m_key_handle.key();
    swap(key_event_handle()); // close this handle
    details::set_or_throw(&ec, ec2, __FUNCTION__, std::move(key));
}

key_event_filter key_event_handle::filter() const noexcept
{ return is_open() ? m_filter : key_event_filter::notify_none; }

bool key_event_handle::is_open() const noexcept { return static_cast<bool>(m_event_handle); }

key_event_handle::native_handle_type key_event_handle::native_handle() const noexcept
{ return reinterpret_cast<native_handle_type>(m_event_handle.get()); }

bool key_event_handle::watch_subtree() const noexcept { return is_open() ? m_watch_subtree : false; }

void key_event_handle::close(std::error_code& ec)
{
    std::error_code ec2;
    key_event_handle tmp(std::move(*this));
    if (close_handle(tmp.m_event_handle.release(), ec2), !ec2) RETURN_RESULT(ec, VOID);

    details::set_or_throw(&ec, ec2, __FUNCTION__, tmp.m_key_handle.key());
}

key_event_handle::native_handle_type key_event_handle::release() noexcept
{
    auto hevent = reinterpret_cast<native_handle_type>(m_event_handle.get());
    return swap(key_event_handle()), hevent;
}

void key_event_handle::swap(key_event_handle& other) noexcept
{
    using std::swap;
    swap(m_watch_subtree, other.m_watch_subtree);
    swap(m_filter, other.m_filter);
    swap(m_key_handle, other.m_key_handle);
    swap(m_event_handle, other.m_event_handle);
}


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

size_t hash_value(const key_event_handle& handle) noexcept
{ return std::hash<key_event_handle::native_handle_type>()(handle.native_handle()); }

} // namespace registry