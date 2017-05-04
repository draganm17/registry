#pragma once

#include <memory>
#include <type_traits>

#include <boost/variant.hpp>


namespace registry {
namespace details {

    template <typename T>
    class possibly_ptr
    {
    private:
        T* do_get(T* val) const noexcept { return val; }

        T* do_get(T& val) const noexcept { return &val; }

        bool do_check(T* val) const noexcept { return static_cast<bool>(val); }

        bool do_check(T &val) const noexcept { return true; }

    public:
        possibly_ptr() noexcept : m_ptr(nullptr) { }

        explicit possibly_ptr(T* ptr) noexcept : m_ptr(ptr) { }

        template <typename TT>
        explicit possibly_ptr(TT&& val) /*noexcept TODO: ... */ : m_ptr(std::forward<TT>(val)) { }

    public:
        explicit operator bool() const noexcept 
        { return boost::apply_visitor([this](auto&& v) { return do_check(v); }, m_ptr); }

        T* operator->() const noexcept 
        { return boost::apply_visitor([this](auto&& v) { return do_get(v); }, m_ptr); }

    public:
        void swap(possibly_ptr& other) noexcept(m_ptr.swap(other.m_ptr)) { m_ptr.swap(other.m_ptr); }

    private:
        boost::variant<T*, std::remove_const_t<T>> m_ptr;
    };

    template <typename T>
    class possibly_shared_ptr
    {
    private:
        T* do_get(T* val) const noexcept { return val; }

        T* do_get(const std::shared_ptr<T>& val) const noexcept { return val.get(); }

        bool do_check(T* val) const noexcept { return static_cast<bool>(val); }

        bool do_check(const std::shared_ptr<T>& val) const noexcept { return static_cast<bool>(val); }

    public:
        possibly_shared_ptr() noexcept : m_ptr(nullptr) { }

        explicit possibly_shared_ptr(T* ptr) noexcept : m_ptr(ptr) { }

        explicit possibly_shared_ptr(const std::shared_ptr<T> &ptr) noexcept : m_ptr(ptr) { }

    public:
        explicit operator bool() const noexcept 
        { return boost::apply_visitor([this](auto&& v) { return do_check(v); }, m_ptr); }

        T* operator->() const noexcept
        { return boost::apply_visitor([this](auto&& v) { return do_get(v); }, m_ptr); }

    public:
        void swap(possibly_shared_ptr& other) noexcept { m_ptr.swap(other.m_ptr); }

    private:
        boost::variant<T*, std::shared_ptr<T>> m_ptr;
    };

    template <typename T>
    class possibly_weak_ptr
    {
        using shared_t = possibly_shared_ptr<T>;

    private:
        possibly_shared_ptr<T> do_lock(T* val) const noexcept { return shared_t(val); }

        possibly_shared_ptr<T> do_lock(const std::weak_ptr<T>& val) const noexcept { return shared_t(val.lock()); }

    public:
        possibly_weak_ptr() noexcept : m_ptr(nullptr) { }

        explicit possibly_weak_ptr(T* ptr) noexcept : m_ptr(ptr) { }

        explicit possibly_weak_ptr(const std::shared_ptr<T>& ptr) noexcept : m_ptr(std::weak_ptr<T>(ptr)) { }

    public:
        possibly_shared_ptr<T> lock() const noexcept
        { return boost::apply_visitor([this](auto&& v) { return do_lock(v); }, m_ptr); }

    public:
        void swap(possibly_weak_ptr& other) noexcept { m_ptr.swap(other.m_ptr); }

    private:
        boost::variant<T*, std::weak_ptr<T>> m_ptr;
    };

    template <typename T>
    inline void swap(possibly_ptr<T>& lhs, possibly_ptr<T>& rhs) noexcept(lhs.swap(rhs)) { lhs.swap(rhs); }

    template <typename T>
    inline void swap(possibly_shared_ptr<T>& lhs, possibly_shared_ptr<T>& rhs) noexcept { lhs.swap(rhs); }

    template <typename T>
    inline void swap(possibly_weak_ptr<T>& lhs, possibly_weak_ptr<T>& rhs) noexcept { lhs.swap(rhs); }

}} // namespace registry::details