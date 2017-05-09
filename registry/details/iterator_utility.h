#pragma once

#include <memory>
#include <type_traits>
#if _HAS_CXX17
#include <variant>
#endif

#if !_HAS_CXX17
#include <boost/variant.hpp>
#endif


#if _HAS_CXX17
#define VARIANT std::variant
#define VISIT   std::visit
#else
#define VARIANT boost::variant
#define VISIT   boost::apply_visitor
#endif


namespace registry {
namespace details {

    template <typename T>
    class possibly_ptr
    {
    private:
        T* do_get(T* val) const noexcept { return val; }

        T* do_get(T& val) const noexcept { return std::addressof(val); }

        bool do_check(T* val) const noexcept { return static_cast<bool>(val); }

        bool do_check(T& val) const noexcept { return true; }

    public:
        possibly_ptr() noexcept : m_ptr(nullptr) { }

        explicit possibly_ptr(T* ptr) noexcept : m_ptr(ptr) { }

        template <typename TT>
        explicit possibly_ptr(TT&& val) noexcept(std::is_nothrow_constructible<T, TT>::value) 
        : m_ptr(std::forward<TT>(val)) { }

    public:
        explicit operator bool() const noexcept { return VISIT([this](auto&& v) { return do_check(v); }, m_ptr); }

        T* operator->() const noexcept { return VISIT([this](auto&& v) { return do_get(v); }, m_ptr); }

    public:
        void swap(possibly_ptr& other) noexcept(noexcept(m_ptr.swap(other.m_ptr))) { m_ptr.swap(other.m_ptr); }

    private:
        VARIANT<T*, std::remove_const_t<T>> m_ptr;
    };

    template <typename T>
    class possibly_shared_ptr
    {
    public:
        possibly_shared_ptr() noexcept : m_ptr(nullptr) { }

        explicit possibly_shared_ptr(T* ptr) noexcept : m_ptr(ptr) { }

        explicit possibly_shared_ptr(const std::shared_ptr<T>& ptr) noexcept : m_ptr(ptr) { }

    public:
        explicit operator bool() const noexcept { return VISIT([](auto&& v) { return static_cast<bool>(v); }, m_ptr); }

        T* operator->() const noexcept { return VISIT([](auto&& v) { return std::addressof(*v); }, m_ptr); }

    public:
        void swap(possibly_shared_ptr& other) noexcept { m_ptr.swap(other.m_ptr); }

    private:
        VARIANT<T*, std::shared_ptr<T>> m_ptr;
    };

    template <typename T>
    class possibly_weak_ptr
    {
    private:
        possibly_shared_ptr<T> do_lock(T* val) const noexcept { return possibly_shared_ptr<T>(val); }

        possibly_shared_ptr<T> do_lock(const std::weak_ptr<T>& val) const noexcept { return possibly_shared_ptr<T>(val.lock()); }

    public:
        possibly_weak_ptr() noexcept : m_ptr(nullptr) { }

        explicit possibly_weak_ptr(T* ptr) noexcept : m_ptr(ptr) { }

        explicit possibly_weak_ptr(const std::shared_ptr<T>& ptr) noexcept : m_ptr(std::weak_ptr<T>(ptr)) { }

    public:
        possibly_shared_ptr<T> lock() const noexcept { return VISIT([this](auto&& v) { return do_lock(v); }, m_ptr); }

    public:
        void swap(possibly_weak_ptr& other) noexcept { m_ptr.swap(other.m_ptr); }

    private:
        VARIANT<T*, std::weak_ptr<T>> m_ptr;
    };

    template <typename T>
    inline void swap(possibly_ptr<T>& lhs, possibly_ptr<T>& rhs) noexcept(noexcept(lhs.swap(rhs))) { lhs.swap(rhs); }

    template <typename T>
    inline void swap(possibly_shared_ptr<T>& lhs, possibly_shared_ptr<T>& rhs) noexcept { lhs.swap(rhs); }

    template <typename T>
    inline void swap(possibly_weak_ptr<T>& lhs, possibly_weak_ptr<T>& rhs) noexcept { lhs.swap(rhs); }

}} // namespace registry::details

#undef VARIANT
#undef VISIT