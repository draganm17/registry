#pragma once

#include <iterator>
#include <type_traits>

#include <registry/details/encoding.h>
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
        // TODO: remove ???
        template <typename Source, 
                  typename = std::enable_if_t<encoding::encoding_type_is<Source, encoding::native_encoding_type>::value>
        >
        static key_name_iterator begin(const Source& name) noexcept
        {
            key_name_iterator it;
            using ST = encoding::string_traits<Source>;
            it.m_key_name = string_view_type(ST::data(name), ST::size(name));
            it.m_element  = string_view_type(ST::data(name), 0);
            return ++it;
        }

        // TODO: remove ???
        template <typename Source, 
                  typename = std::enable_if_t<encoding::encoding_type_is<Source, encoding::native_encoding_type>::value>
        >
        static key_name_iterator end(const Source& name) noexcept
        {
            key_name_iterator it;
            using ST = encoding::string_traits<Source>;
            it.m_key_name = string_view_type(ST::data(name), ST::size(name));
            it.m_element =  string_view_type(ST::data(name) + ST::size(name), 0);
            return it;
        }

        static key_name_iterator begin(const string_type::value_type* first, const string_type::value_type* last) noexcept
        {
            key_name_iterator it;
            it.m_key_name = string_view_type(first, last - first);
            it.m_element  = string_view_type(first, 0);
            return ++it;
        }

        static key_name_iterator end(const string_type::value_type* first, const string_type::value_type* last) noexcept
        {
            key_name_iterator it;
            it.m_key_name = string_view_type(first, last - first);
            it.m_element  = string_view_type(last, 0);
            return ++it;
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
            auto first = m_element.data() + m_element.size();
            const auto end = m_key_name.data() + m_key_name.size();
            for (;  first != end && *first == separator;  ++first);

            auto last = first;
            for (; last != end && *last != separator; ++last);

            m_element = string_view_type(first, last - first);
            return *this;
        }

        key_name_iterator operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }

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