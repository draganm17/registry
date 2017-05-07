#pragma once

#include <iterator>

#include <registry/types.h>


namespace registry {
namespace details {

    // NOTE: key_name_iterator is able to process names with multiple redundant separators.
    class key_name_iterator
    {
        static constexpr auto separator = string_type::value_type('\\');

    public:
        using value_type =        string_view_type;
        using difference_type =   std::ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        // NOTE: 'name' may not be null-terminated
        static key_name_iterator begin(string_view_type name) noexcept
        {
            key_name_iterator it;
            it.m_key_name = name;
            it.m_element = string_view_type(name.data(), 0);
            return ++it;
        }

        // NOTE: 'name' may not be null-terminated
        static key_name_iterator end(string_view_type name) noexcept
        {
            key_name_iterator it;
            it.m_key_name = name;
            it.m_element = string_view_type(name.data() + name.size(), 0);
            return it;
        }

    public:
        bool operator==(const key_name_iterator& rhs) const noexcept 
        { return m_element.data() == rhs.m_element.data() && m_element.size() == rhs.m_element.size(); }

        bool operator!=(const key_name_iterator& rhs) const noexcept { return !(*this == rhs); }

        reference operator*() const noexcept { return m_element; }

        pointer operator->() const noexcept { return &m_element; }

    public:
        key_name_iterator& operator++() noexcept
        {
            auto first = m_element.end();
            const auto end = m_key_name.end();
            for (; first != end && *first == separator; ++first);

            auto last = first;
            for (; last != end && *last != separator; ++last);

            m_element = string_view_type(first, last - first);
            return *this;
        }

        key_name_iterator operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }

        key_name_iterator& operator--() noexcept
        {
            auto last = m_element.begin() - 1;
            for (; *last == separator;  --last);

            auto first = last;
            auto rbeg = m_key_name.begin() - 1;
            for (; first != rbeg && *first != separator; --first);

            ++first; ++last;
            m_element = string_view_type(first, last - first);
            return *this;
        }

        key_name_iterator operator--(int) noexcept { auto tmp = *this; --*this; return tmp; }

    public:
        void swap(key_name_iterator& other) noexcept
        {
            using std::swap;
            swap(m_element, other.m_element);
            swap(m_key_name, other.m_key_name);
        }

    private:
        string_view_type  m_element;
        string_view_type  m_key_name;
    };

}} // namespace registry::details