#include <algorithm>
#include <cassert>
#include <locale>
#include <Windows.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>

#include <registry/details/utils.impl.h>
#include <registry/key_path.h>


namespace registry {

//------------------------------------------------------------------------------------//
//                                 class key_path                                     //
//------------------------------------------------------------------------------------//

key_path& key_path::append_impl(string_view_type subkey)
{
    const bool add_slash = !(begin() == end() || m_name.back() == separator ||
                             subkey.empty() || subkey.front() == separator);


    m_name.reserve(m_name.size() + subkey.size() + static_cast<int>(add_slash));
    m_name.append(add_slash ? TEXT("\\") : TEXT("")).append(subkey.data(), subkey.size());
    return *this;
}

key_path key_path::from_key_id(key_id id) { return key_path(details::key_id_to_string(id)); }

key_path::key_path(string_view_type name, view view)
    : m_view(view)
    , m_name(static_cast<string_type>(name))
{ }

const string_type& key_path::key_name() const noexcept { return m_name; }

view key_path::key_view() const noexcept { return m_view; }

key_path key_path::root_key() const 
{ return has_root_key() ? key_path(*begin(), m_view) : key_path(string_view_type(), m_view); }

key_id key_path::root_key_id() const { return details::key_id_from_string(*begin()); }

key_path key_path::leaf_key() const 
{ return has_leaf_key() ? key_path(*--end(), m_view) : key_path(string_view_type(), m_view); }

key_path key_path::parent_key() const
{
    auto first = begin(), last = end();
    key_path path(string_view_type(), m_view);

    if (first != last && first != --last) {
        for (; first != last; ++first) path.append(*first);
    }
    return path;
}

bool key_path::has_root_key() const noexcept { return begin() != end(); }

bool key_path::has_leaf_key() const noexcept { return begin() != end(); }

bool key_path::has_parent_key() const noexcept
{
    auto beg_it = begin(), end_it = end();
    return beg_it != end_it && ++beg_it != end_it;
}

bool key_path::is_absolute() const noexcept
{
    const auto beg_it = begin(), end_it = end();
    return beg_it != end_it && beg_it->data() == m_name.data() && 
           details::key_id_from_string(*beg_it) != key_id::unknown;
}

bool key_path::is_relative() const noexcept { return !is_absolute(); }

int key_path::compare(const key_path& other) const noexcept
{
    if (m_view != other.m_view) {
        return m_view < other.m_view ? -1 : 1;
    }

    iterator beg_1 = begin(), end_1 = end();
    iterator beg_2 = other.begin(), end_2 = other.end();
    for (; beg_1 != end_1 && beg_2 != end_2; ++beg_1, ++beg_2) {
        if (boost::ilexicographical_compare(*beg_1, *beg_2)) return -1;
        if (boost::ilexicographical_compare(*beg_2, *beg_1)) return  1;
    }
    return int(beg_2 == end_2) - int(beg_1 == end_1);
}

key_path::iterator key_path::begin() const noexcept
{
    iterator it;
    it.m_value = string_view_type(m_name.data(), 0);
    it.m_key_name_view = string_view_type(m_name.data(), m_name.size());
    return ++it;
}

key_path::iterator key_path::end() const noexcept
{
    iterator it;
    it.m_value = string_view_type(m_name.data() + m_name.size(), 0);
    it.m_key_name_view = string_view_type(m_name.data(), m_name.size());
    return it;
}

key_path& key_path::assign(string_view_type name, view view)
{
    m_name.assign(name.data(), name.size());
    m_view = view;
    return *this;
}

key_path& key_path::append(const key_path& subkey)
{
    if (subkey.m_view != view::view_default) m_view = subkey.m_view;
    for (auto it = subkey.begin(); it != subkey.end(); ++it) append(*it);
    return *this;
}

key_path& key_path::concat(string_view_type str)
{
    m_name.append(str.data(), str.size());
    return *this;
}

key_path& key_path::remove_leaf_key()
{
    assert(has_leaf_key());

    auto it = --end();
    m_name.resize((it != begin()) ? (--it, it->data() - m_name.data() + it->size()) : 0);
    return *this;
}

key_path& key_path::replace_leaf_key(string_view_type replacement)
{
    assert(has_leaf_key());
    return remove_leaf_key().append(replacement);
}

void key_path::swap(key_path& other) noexcept
{
    using std::swap;
    swap(m_view, other.m_view);
    swap(m_name, other.m_name);
}

bool key_path::iterator::operator==(const iterator& rhs) const noexcept
{
    return m_value.data() == rhs.m_value.data() && m_value.size() == rhs.m_value.size();
}

bool key_path::iterator::operator!=(const iterator& rhs) const noexcept { return !(*this == rhs); }


//------------------------------------------------------------------------------------//
//                           class key_path::iterator                                 //
//------------------------------------------------------------------------------------//

key_path::iterator::reference key_path::iterator::operator*() const noexcept
{
    // TODO: is end iterator assert

    return m_value;
}

key_path::iterator::pointer key_path::iterator::operator->() const noexcept
{
    // TODO: is end iterator assert

    return &m_value;
}

key_path::iterator& key_path::iterator::operator++() noexcept
{
    // TODO: is end iterator assert

    const auto end = m_key_name_view.end();

    auto first = m_value.end();
    for (; first != end && *first == separator; ++first);

    auto last = first;
    for (; last != end && *last != separator; ++last);

    m_value = string_view_type(first, last - first);
    return *this;
}

key_path::iterator key_path::iterator::operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }

key_path::iterator& key_path::iterator::operator--() noexcept
{
    // TODO: is begin iterator assert

    const auto rbegin = m_key_name_view.begin() - 1;

    auto last = m_value.begin() - 1;
    for (; last != rbegin && *last == separator; --last);

    auto first = last;
    for (; first != rbegin && *first != separator; --first);

    m_value = string_view_type(++first, ++last - first);
    return *this;
}

key_path::iterator key_path::iterator::operator--(int) noexcept { auto tmp = *this; --*this; return tmp; }

void key_path::iterator::swap(iterator& other) noexcept
{
    m_value.swap(other.m_value);
    m_key_name_view.swap(other.m_key_name_view);
}


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

size_t hash_value(const key_path& path) noexcept
{
    const auto locale = std::locale();
    size_t hash = std::hash<view>()(path.key_view());
    for (auto it = path.begin(); it != path.end(); ++it) {
        std::for_each(it->begin(), it->end(), [&](auto c) { boost::hash_combine(hash, std::tolower(c, locale)); });
    }
    return hash;
}

}  // namespace registry