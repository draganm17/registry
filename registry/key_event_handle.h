/** @file */
#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <system_error>

#include <registry/key_handle.h>
#include <registry/types.h>


namespace registry
{
    class key;

    // TODO: ...
    enum class key_event_status : uint32_t
    {
        /*! TODO: ... */
        signalled =  0x00000000,

        /*! TODO: ... */
        timeout =    0x00000102,

        /*! TODO: ... */
        failed =     0xFFFFFFFF
    };

    // TODO: ...
    enum class key_event_filter : uint32_t
    {
        /*! TODO: ... */
        notify_none =               0x00000000,

        /*! Notify the caller if a subkey is added or deleted. */
        notify_change_name =        0x00000001,

        /*! Notify the caller of changes to the attributes of the key, such as the security descriptor information. */
        notify_change_attributes =  0x00000002,

        /*! Notify the caller of changes to a value of the key. This can include adding or deleting a value, or 
        changing an existing value. */
        notify_change_last_set =    0x00000004,

        /*! Notify the caller of changes to the security descriptor of the key. */
        notify_change_security =    0x00000008,

        /*! TODO: ... */
        notify_all =                0x0000000F

        // REG_NOTIFY_THREAD_AGNOSTIC ???
    };

    //------------------------------------------------------------------------------------//
    //                             class key_event_handle                                 //
    //------------------------------------------------------------------------------------//

    // TODO: ...
    class key_event_handle
    {
    public:
        // TODO: ...
        using native_handle_type = void*;

    private:
        struct close_handle_t { void operator()(void*) const noexcept; };

    private:
        key_event_status wait_impl(
            const char* caller,
            std::chrono::milliseconds rel_time, 
            std::error_code& ec = throws()) const;

    public:
        // TODO: ...
        key_event_handle() noexcept = default;

        // TODO: ...
        key_event_handle(const key_event_handle&) = delete;

        // TODO: ...
        key_event_handle(key_event_handle&&) noexcept = default;

        // TODO: ...
        key_event_handle& operator=(const key_event_handle&) = delete;

        // TODO: ...
        key_event_handle& operator=(key_event_handle&&) noexcept = default;

        // TODO: ...
        key_event_handle(const key& key, key_event_filter filter, 
                         bool watch_subtree, std::error_code& ec = throws());

        // TODO: ...
        key_event_handle(key_handle handle, key_event_filter filter, 
                         bool watch_subtree, std::error_code& ec = throws());

        // TODO: ...
        ~key_event_handle() noexcept = default;

    public:
        // TODO: ...
        key_event_filter filter() const noexcept;

        // TODO: ...
        bool is_open() const noexcept;

        // TODO: ...
        native_handle_type native_handle() const noexcept;

        // TODO: ...
        bool watch_subtree() const noexcept;

    public:
        // TODO: ...
        void wait(std::error_code& ec = throws()) const;

        // TODO: ...
        template< class Rep, class Period >
        key_event_status wait_for(const std::chrono::duration<Rep, Period>& rel_time, 
                                  std::error_code& ec = throws()) const;

        // TODO: ...
        template< class Clock, class Duration >
        key_event_status wait_until(const std::chrono::time_point<Clock, Duration>& abs_time, 
                                    std::error_code& ec = throws()) const;

    public:
        // TODO: ...
        void close(std::error_code& ec = throws());

        // TODO: ...
        // post cond: !is_open() 
        native_handle_type release() noexcept;  // TODO: ???

        //! Swaps the contents of `*this` and `other`.
        void swap(key_event_handle& other) noexcept;

    private:
        bool                                   m_watch_subtree;
        key_event_filter                       m_filter;
        key_handle                             m_key_handle;
        std::unique_ptr<void, close_handle_t>  m_event_handle;
    };


    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    constexpr key_event_filter operator&(key_event_filter lhs, key_event_filter rhs) noexcept;

    constexpr key_event_filter operator|(key_event_filter lhs, key_event_filter rhs) noexcept;

    constexpr key_event_filter operator^(key_event_filter lhs, key_event_filter rhs) noexcept;

    constexpr key_event_filter operator~(key_event_filter lhs) noexcept;

    key_event_filter& operator&=(key_event_filter& lhs, key_event_filter rhs) noexcept;

    key_event_filter& operator|=(key_event_filter& lhs, key_event_filter rhs) noexcept;

    key_event_filter& operator^=(key_event_filter& lhs, key_event_filter rhs) noexcept;

    bool operator==(const key_event_handle& lhs, const key_event_handle& rhs) noexcept;

    bool operator!=(const key_event_handle& lhs, const key_event_handle& rhs) noexcept;

    bool operator<(const key_event_handle& lhs, const key_event_handle& rhs) noexcept;

    bool operator>(const key_event_handle& lhs, const key_event_handle& rhs) noexcept;

    bool operator<=(const key_event_handle& lhs, const key_event_handle& rhs) noexcept;

    bool operator>=(const key_event_handle& lhs, const key_event_handle& rhs) noexcept;

    //! Calculates a hash value for a `key_handle` object.
    /*!
    @return A hash value such that if for two handles, `h1 == h2` then `hash_value(h1) == hash_value(h2)`.
    */
    size_t hash_value(const key_event_handle& handle) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key_event_handle& lhs, key_event_handle& rhs) noexcept;

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

    inline constexpr key_event_filter operator&(key_event_filter lhs, key_event_filter rhs) noexcept
    { return static_cast<key_event_filter>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)); }

    inline constexpr key_event_filter operator|(key_event_filter lhs, key_event_filter rhs) noexcept
    { return static_cast<key_event_filter>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)); }

    inline constexpr key_event_filter operator^(key_event_filter lhs, key_event_filter rhs) noexcept
    { return static_cast<key_event_filter>(static_cast<uint32_t>(lhs) ^ static_cast<uint32_t>(rhs)); }

    inline constexpr key_event_filter operator~(key_event_filter lhs) noexcept
    { return static_cast<key_event_filter>(~static_cast<uint32_t>(lhs)); }

    inline key_event_filter& operator&=(key_event_filter& lhs, key_event_filter rhs) noexcept { return lhs = lhs & rhs; }

    inline key_event_filter& operator|=(key_event_filter& lhs, key_event_filter rhs) noexcept { return lhs = lhs | rhs; }

    inline key_event_filter& operator^=(key_event_filter& lhs, key_event_filter rhs) noexcept { return lhs = lhs ^ rhs; }

    inline bool operator==(const key_event_handle& lhs, const key_event_handle& rhs) noexcept
    { return lhs.native_handle() == rhs.native_handle(); }

    inline bool operator!=(const key_event_handle& lhs, const key_event_handle& rhs) noexcept { return !(lhs == rhs); }

    inline bool operator<(const key_event_handle& lhs, const key_event_handle& rhs) noexcept
    { return lhs.native_handle() < rhs.native_handle(); }

    inline bool operator>(const key_event_handle& lhs, const key_event_handle& rhs) noexcept
    { return lhs.native_handle() > rhs.native_handle(); }

    inline bool operator<=(const key_event_handle& lhs, const key_event_handle& rhs) noexcept { return !(lhs > rhs); }

    inline bool operator>=(const key_event_handle& lhs, const key_event_handle& rhs) noexcept { return !(lhs < rhs); }

    inline void swap(key_event_handle& lhs, key_event_handle& rhs) noexcept { lhs.swap(rhs); }

    inline void key_event_handle::wait(std::error_code& ec) const 
    { wait_impl(__FUNCTION__, std::chrono::milliseconds::max(), ec); }

    template< class Rep, class Period >
    inline key_event_status key_event_handle::wait_for(const std::chrono::duration<Rep, Period>& rel_time,
                                                       std::error_code& ec) const
    { return wait_impl(__FUNCTION__, std::chrono::duration_cast<std::chrono::milliseconds>(rel_time), ec); }

    template< class Clock, class Duration >
    inline key_event_status key_event_handle::wait_until(const std::chrono::time_point<Clock, Duration>& abs_time,
                                                         std::error_code& ec) const
    { return wait_impl(__FUNCTION__, std::chrono::duration_cast<std::chrono::milliseconds>(abs_time - Clock::now()), ec); }

} // namespace registry