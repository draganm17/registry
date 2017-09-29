#pragma once

#include <iterator>
#include <string_view>


namespace registry {
namespace details {

    // NOTE: key_name_iterator is able to process names with multiple redundant separators.
    class key_name_iterator
    {
        static constexpr auto separator = string_type::value_type('\\');

    public:
        using value_type =        std::wstring_view;
        using difference_type =   std::ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        static key_name_iterator begin(std::wstring_view name) noexcept
        {
            key_name_iterator it;
            it.m_key_name = string_view_type(name.data(), name.size());
            it.m_element  = string_view_type(name.data(), 0);
            return ++it;
        }

        static key_name_iterator end(std::wstring_view name) noexcept
        {
            key_name_iterator it;
            it.m_key_name = string_view_type(name.data(), name.size());
            it.m_element  = string_view_type(name.data() + name.size(), 0);
            return ++it;
        }

    public:
        bool operator==(const key_name_iterator& rhs) const noexcept 
        {
            return m_element.data() == rhs.m_element.data() && 
                   m_element.size() == rhs.m_element.size();
        }

        bool operator!=(const key_name_iterator& rhs) const noexcept
        {
            return !(*this == rhs);
        }

        reference operator*() const noexcept
        {
            return m_element;
        }

        pointer operator->() const noexcept
        {
            return &m_element;
        }

    public:
        key_name_iterator& operator++() noexcept
        {
            auto first = m_element.data() + m_element.size();
            const auto end = m_key_name.data() + m_key_name.size();
            for (;  first != end && *first == separator;  ++first);

            auto last = first;
            for (; last != end && *last != separator; ++last);

            m_element = string_view_type(first, last - first);
            return *this;
        }

        key_name_iterator operator++(int) noexcept
        {
            auto tmp = *this; ++*this; return tmp;
        }

        key_name_iterator& operator--() noexcept
        {
            auto last = m_element.data() - 1;
            for (; *last == separator; --last);

            auto first = last;
            auto rbeg = m_key_name.data() - 1;
            for (; first != rbeg && *first != separator; --first);

            ++first; ++last;
            m_element = string_view_type(first, last - first);
            return *this;
        }

        key_name_iterator operator--(int) noexcept
        {
            auto tmp = *this; --*this; return tmp;
        }

    public:
        void swap(key_name_iterator& other) noexcept
        {
            using std::swap;
            swap(m_element, other.m_element);
            swap(m_key_name, other.m_key_name);
        }

    private:
        std::wstring_view  m_element;

        std::wstring_view  m_key_name;
    };

}} // namespace registry::details