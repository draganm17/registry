#pragma once

#include <memory>

#include <boost/variant.hpp>


namespace registry {

    class key;

namespace details {

    class possibly_weak_key_ptr
    {
    private:
        void do_lock(const key* val) noexcept { }

        void do_lock(const std::weak_ptr<key> &val) noexcept { m_shared_ptr = val.lock(); }

        void do_unlock(const key* val) noexcept { }

        void do_unlock(const std::weak_ptr<key> &val) noexcept { m_shared_ptr.reset(); }

        const key* do_get(const key* val) const noexcept { return val; }

        const key* do_get(const std::weak_ptr<key> &val) const noexcept { return m_shared_ptr.get(); }

    public:
        possibly_weak_key_ptr() noexcept : m_ptr(nullptr) { }

        possibly_weak_key_ptr(const key* ptr) noexcept : m_ptr(ptr) { }

        possibly_weak_key_ptr(const std::shared_ptr<key> &ptr) noexcept : m_ptr(std::weak_ptr<key>(ptr)) { }

    public:
        void lock() noexcept { boost::apply_visitor([this](auto&& v) { do_lock(v); }, m_ptr); }

        void unlock() noexcept { boost::apply_visitor([this](auto&& v) { do_unlock(v); }, m_ptr); }

    public:
        const key* get() const noexcept { return boost::apply_visitor([this](auto&& v) { return do_get(v); }, m_ptr); }

    private:
        std::shared_ptr<key> m_shared_ptr;
        boost::variant<const key*, std::weak_ptr<key>> m_ptr;
    };

}} // namespace registry::details