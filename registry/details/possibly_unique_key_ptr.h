#pragma once

#include <boost/variant.hpp>

#include <registry/key.h>


namespace registry {
namespace details {

    class possibly_unique_key_ptr
    {
    private:
        const key* do_get(const key* val) const noexcept { return val; }

        const key* do_get(const key &val) const noexcept { return &val; }

    public:
        possibly_unique_key_ptr() noexcept : m_ptr(nullptr) { }

        possibly_unique_key_ptr(const key* ptr) noexcept : m_ptr(ptr) { assert(ptr); }

        possibly_unique_key_ptr(key&& val) noexcept : m_ptr(std::move(val)) { }

    public:
        const key* get() const noexcept { return boost::apply_visitor([this](auto&& v) { return do_get(v); }, m_ptr); }

    private:
        boost::variant<const key*, key> m_ptr;
    };

}} // namespace registry::details