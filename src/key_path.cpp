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

key_path& key_path::do_append(string_view_type src)
{
    m_name.reserve(m_name.size() + src.size());
    auto first = src.begin(), last = src.end();
    
    // append each component of 'src' to the key name
    while (first != last)
    {
        m_name.push_back(key_path::separator);
        for (; first != last && *first == key_path::separator; ++first);
        for (; first != last && *first != key_path::separator; ++first) m_name.push_back(*first);
    }

    // remove trailing separator from the key name
    if (m_name.size() && m_name.back() == key_path::separator) m_name.pop_back();

    return *this;
}

key_path& key_path::do_replace_leaf_path(string_view_type src)
{
    assert(has_leaf_path());
    return remove_leaf_path().append(src);
}

key_path key_path::from_key_id(key_id id) { return key_path(details::key_id_to_string(id)); }

key_path::key_path(view view) noexcept
    : m_view(view)
{ }

key_path::key_path(string_view_type name, view view)
    : m_view(view)
{
    do_append(name);
}

const string_type& key_path::key_name() const noexcept { return m_name; }

view key_path::key_view() const noexcept { return m_view; }

key_path key_path::root_path() const { return key_path(details::key_id_to_string(root_key_id()), m_view); }

key_id key_path::root_key_id() const noexcept
{ return !m_name.empty() ? details::key_id_from_string(*begin()) : key_id::unknown; }

key_path key_path::leaf_path() const 
{ return has_leaf_path() ? key_path(*--end(), m_view) : key_path(string_view_type(), m_view); }

key_path key_path::parent_path() const
{
    auto first = begin(), last = end();
    key_path path(string_view_type(), m_view);

    if (first != last && first != --last) {
        for (; first != last; ++first) path.append(*first);
    }
    return path;
}

key_path key_path::relative_path() const
{
    if (!has_root_path()) return *this;
    
    const auto it = ++begin();
    return it != end() ? key_path(it->data(), m_view)
                       : key_path(string_view_type(), m_view);
}

bool key_path::has_root_path() const noexcept { return root_key_id() != key_id::unknown; }

bool key_path::has_leaf_path() const noexcept { return begin() != end(); }

bool key_path::has_parent_path() const noexcept
{
    auto beg_it = begin(), end_it = end();
    return beg_it != end_it && ++beg_it != end_it;
}

bool key_path::has_relative_path() const noexcept 
{
    auto first = begin(), last = end();
    const bool has_rp = has_root_path();
    return (has_rp && ++first != last) || (!has_rp && first != last);
}

bool key_path::is_absolute() const noexcept { return has_root_path(); }

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
    it.m_value = string_view_type(m_name.data() - 1, 0);
    it.m_key_name_view = static_cast<string_view_type>(m_name);
    return ++it;
}

key_path::iterator key_path::end() const noexcept
{
    iterator it;
    it.m_value = string_view_type(m_name.data() + m_name.size(), 0);
    it.m_key_name_view = static_cast<string_view_type>(m_name);
    return it;
}

key_path& key_path::assign(view view)
{
    m_name.clear();
    m_view = view;
    return *this;
}

key_path& key_path::assign(string_view_type name, view view)
{
    m_name.assign(name.data(), name.size());
    m_view = view;
    return *this;
}

key_path& key_path::append(const key_path& subkey)
{
    do_append(subkey.m_name);
    if (subkey.m_view != view::view_default) m_view = subkey.m_view;
    return *this;
}

key_path& key_path::concat(string_view_type str)
{
    // TODO: ...

    //do_concat(m_name, str);
    //m_name.append(str.data(), str.size());
    return *this;
}

key_path& key_path::remove_leaf_path()
{
    // TODO: check that function

    if (!m_name.empty()) {
        auto it = --end();
        m_name.resize((it != begin()) ? (--it, it->data() - m_name.data() + it->size()) : 0);
    }
    return *this;
}

key_path& key_path::replace_leaf_path(const key_path& replacement)
{
    do_replace_leaf_path(replacement.m_name);
    if (replacement.m_view != view::view_default) m_view = replacement.m_view;
    return *this;
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
    auto first = m_value.end() + 1, last = first;
    for (; last != end && *last != separator; ++last);

    m_value = string_view_type(first, last - first);
    return *this;
}

key_path::iterator key_path::iterator::operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }

key_path::iterator& key_path::iterator::operator--() noexcept
{
    // TODO: is begin iterator assert

    // TODO: ...

    //const auto rbeg = m_key_name_view.begin() - 1;
    //auto rfirst = m_value.end() - 1, rlast = rfirst;
    //for (; rlast != rbeg && *rlast != separator; --rlast);

    //m_value = string_view_type(rlast + 1, rfirst - rlast);
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