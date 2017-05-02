#include <algorithm>
#include <Windows.h>

#include <registry/details/utils.impl.h>
#include <registry/key_event.h>


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
    return details::set_or_throw(&ec, ec2, caller), key_event_status::failed;
}

key_event::key_event(const key_path& path, key_event_options options, std::error_code& ec)
    : m_options(options)
{
    assert((options & key_event_options::notify_all) != key_event_options::notify_none);

    std::error_code ec2;
    m_key = key(open_only_tag{}, path, access_rights::notify, ec2);
    if (!ec2)
    {
        LRESULT rc{};
        HANDLE hevent;
        const HKEY hkey = reinterpret_cast<HKEY>(m_key.native_handle());
        const DWORD filter = static_cast<DWORD>(options & key_event_options::notify_all);
        const BOOL watch_subtree = static_cast<BOOL>(options & key_event_options::watch_subtree);

        if ((hevent = CreateEvent(nullptr, TRUE, FALSE, nullptr)) &&
            !(rc = RegNotifyChangeKeyValue(hkey, watch_subtree, filter, hevent, TRUE)))
        {
            RETURN_RESULT(ec, VOID);
        }
        ec2 = std::error_code(rc ? rc : GetLastError(), std::system_category());
    }

    swap(key_event());
    details::set_or_throw(&ec, ec2, __FUNCTION__, path);
}

key_event_options key_event::options() const noexcept
{ 
    // TODO: ...
    return key_event_options{};
    //return valid() ? m_filter : key_event_filter::notify_none;
}

key_event::native_handle_type key_event::native_handle() const noexcept
{ return reinterpret_cast<native_handle_type>(m_event_handle.get()); }

bool key_event::valid() const noexcept { return static_cast<bool>(m_event_handle); }

void key_event::swap(key_event& other) noexcept
{
    using std::swap;
    swap(m_options, other.m_options);
    swap(m_key, other.m_key);
    swap(m_event_handle, other.m_event_handle);
}

} // namespace registry