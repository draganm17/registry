#include <algorithm>
#include <Windows.h>

#include <registry/details/utils.impl.h>
#include <registry/key_event.h>
#include <registry/key_handle.h>


namespace {

using namespace registry;

void close_handle(key_event::native_handle_type handle, std::error_code& ec) noexcept
{
    ec.clear();
    if (!(HANDLE)handle) return;
    const LSTATUS rc = CloseHandle((HANDLE)handle) ? ERROR_SUCCESS : GetLastError();
    if (rc != ERROR_SUCCESS) ec = std::error_code(rc, std::system_category());
}

}


namespace registry {

void key_event::close_handle_t::operator()(void* handle) const noexcept
{ CloseHandle(reinterpret_cast<HANDLE>(handle)); }

key_event_status key_event::wait_impl(const char* caller, std::chrono::milliseconds rel_time, std::error_code& ec) const
{
    assert(valid());  // TODO: ???

    using rep_t = std::chrono::milliseconds::rep;
    const DWORD rc = WaitForSingleObject(reinterpret_cast<HANDLE>(m_event_handle.get()), 
                                         (DWORD)std::min(std::max(rel_time.count(), rep_t(0)), rep_t(INFINITE)));

    if (rc == WAIT_TIMEOUT) RETURN_RESULT(ec, key_event_status::timeout);
    if (rc == WAIT_OBJECT_0) RETURN_RESULT(ec, key_event_status::signalled);

    const std::error_code ec2(GetLastError(), std::system_category());
    return details::set_or_throw(&ec, ec2, caller, m_key_handle.path()), key_event_status::failed;
}

key_event::key_event(const key_path& path, key_event_filter filter, bool watch_subtree, std::error_code& ec)
{
    std::error_code ec2;
    key_handle handle(path, access_rights::notify, ec2);
    if (!ec2 && (swap(key_event(std::move(handle), filter, watch_subtree, ec2)), !ec2)) RETURN_RESULT(ec, VOID);

    details::set_or_throw(&ec, ec2, __FUNCTION__, path);
}

key_event::key_event(key_handle handle, key_event_filter filter, bool watch_subtree, std::error_code& ec)
    : m_watch_subtree(watch_subtree)
    , m_filter(filter)
    , m_key_handle(std::move(handle))
{
    // TODO: check (and refactor ???)

    if (filter == key_event_filter::notify_none) {
        swap(key_event());
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
    key_event tmp(std::move(*this));
    details::set_or_throw(&ec, ec2, __FUNCTION__, tmp.path());
}

key_path key_event::path() const { return valid() ? m_key_handle.path() : key_path(); }

key_event_filter key_event::filter() const noexcept
{ return valid() ? m_filter : key_event_filter::notify_none; }

bool key_event::watch_subtree() const noexcept { return valid() ? m_watch_subtree : false; }

key_event::native_handle_type key_event::native_handle() const noexcept
{ return reinterpret_cast<native_handle_type>(m_event_handle.get()); }

bool key_event::valid() const noexcept { return static_cast<bool>(m_event_handle); }

void key_event::swap(key_event& other) noexcept
{
    using std::swap;
    swap(m_watch_subtree, other.m_watch_subtree);
    swap(m_filter, other.m_filter);
    swap(m_key_handle, other.m_key_handle);
    swap(m_event_handle, other.m_event_handle);
}

} // namespace registry