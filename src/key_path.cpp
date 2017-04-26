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
    for (auto it = details::key_name_iterator::begin(src); it != details::key_name_iterator::end(src); ++it)
    {
        if (!m_name.empty()) m_name.push_back(key_path::separator);
        m_name.append(it->data(), it->size());
    }
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

key_path key_path::root_path() const
{
    const auto it = details::key_name_iterator::begin(m_name);
    return (it == details::key_name_iterator::end(m_name) ||
            details::key_id_from_string(*it) == key_id::unknown) ? key_path(m_view)
                                                                 : key_path(*it, m_view);
}

key_id key_path::root_key_id() const noexcept
{
    const auto it = details::key_name_iterator::begin(m_name);
    return it == details::key_name_iterator::end(m_name) ? key_id::unknown
                                                         : details::key_id_from_string(*it);
}

key_path key_path::leaf_path() const 
{
    auto it = details::key_name_iterator::end(m_name);
    return (it == details::key_name_iterator::begin(m_name)) ? key_path(m_view)
                                                             : key_path(*--it, m_view);
}

key_path key_path::parent_path() const
{
    auto it = details::key_name_iterator::begin(m_name);
    const auto last = details::key_name_iterator::end(m_name);
    return (it == last || ++it == last) ? key_path(m_view)
                                        : key_path(it->data(), m_view);
}

key_path key_path::relative_path() const
{
    if (!has_root_path()) return *this;
    const auto it = ++details::key_name_iterator::begin(m_name);
    return it == details::key_name_iterator::end(m_name) ? key_path(m_view)
                                                         : key_path(it->data(), m_view);
}

bool key_path::has_root_path() const noexcept { return root_key_id() != key_id::unknown; }

bool key_path::has_leaf_path() const noexcept 
{ return details::key_name_iterator::begin(m_name) != details::key_name_iterator::end(m_name); }

bool key_path::has_parent_path() const noexcept
{ return std::distance(details::key_name_iterator::begin(m_name), details::key_name_iterator::end(m_name)) > 1; }

bool key_path::has_relative_path() const noexcept 
{
    const bool has_root = has_root_path();
    auto first = details::key_name_iterator::begin(m_name);
    const auto last = details::key_name_iterator::end(m_name);
    return (has_root && ++first != last) || (!has_root && first != last);
}

bool key_path::is_absolute() const noexcept { return has_root_path(); }

bool key_path::is_relative() const noexcept { return !is_absolute(); }

int key_path::compare(const key_path& other) const noexcept
{
    if (m_view != other.m_view) {
        return m_view < other.m_view ? -1 : 1;
    }

    auto beg_1 = details::key_name_iterator::begin(m_name);
    const auto end_1 = details::key_name_iterator::end(m_name);
    auto beg_2 = details::key_name_iterator::begin(other.m_name);
    const auto end_2 = details::key_name_iterator::end(other.m_name);

    for (; beg_1 != end_1 && beg_2 != end_2; ++beg_1, ++beg_2) {
        if (boost::ilexicographical_compare(*beg_1, *beg_2)) return -1;
        if (boost::ilexicographical_compare(*beg_2, *beg_1)) return  1;
    }
    return int(beg_2 == end_2) - int(beg_1 == end_1);
}

key_path::iterator key_path::begin() const noexcept
{
    iterator it;
    it.m_path_ptr = this;
    it.m_name_iterator = details::key_name_iterator::begin(m_name);
    return ++it;
}

key_path::iterator key_path::end() const noexcept
{
    iterator it;
    it.m_path_ptr = this;
    it.m_name_iterator = details::key_name_iterator::end(m_name);
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

    auto first = details::key_name_iterator::begin(m_name);
    auto last = details::key_name_iterator::end(m_name);

    if (first != last) {
        auto it = --last;
        m_name.resize((it != first) ? (--it, it->data() - first->data() + it->size()) : 0);
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
    // TODO: ...
    return 0;

    //return m_element == rhs.m_element;
}

bool key_path::iterator::operator!=(const iterator& rhs) const noexcept { return !(*this == rhs); }


//------------------------------------------------------------------------------------//
//                           class key_path::iterator                                 //
//------------------------------------------------------------------------------------//

key_path::iterator::reference key_path::iterator::operator*() const noexcept
{
    assert(m_name_iterator != details::key_name_iterator::end(m_path_ptr->key_name()));
    return m_element;
}

key_path::iterator::pointer key_path::iterator::operator->() const noexcept
{
    assert(m_name_iterator != details::key_name_iterator::end(m_path_ptr->key_name()));
    return &m_element;
}

key_path::iterator& key_path::iterator::operator++()
{
    assert(m_name_iterator != details::key_name_iterator::end(m_path_ptr->key_name()));
    
    const auto name_end = details::key_name_iterator::end(m_path_ptr->key_name());
    if (++m_name_iterator != name_end) m_element = key_path(*m_name_iterator, m_path_ptr->key_view());
    return *this;
}

key_path::iterator key_path::iterator::operator++(int) { auto tmp = *this; ++*this; return tmp; }

key_path::iterator& key_path::iterator::operator--()
{
    assert(m_name_iterator != details::key_name_iterator::begin(m_path_ptr->key_name()));

    const auto name_beg = details::key_name_iterator::begin(m_path_ptr->key_name());
    if (m_name_iterator-- != name_beg) m_element = key_path(*m_name_iterator, m_path_ptr->key_view());
    return *this;
}

key_path::iterator key_path::iterator::operator--(int) { auto tmp = *this; --*this; return tmp; }

void key_path::iterator::swap(iterator& other) noexcept
{
    using std::swap;
    swap(m_element, other.m_element);
    swap(m_path_ptr, other.m_path_ptr);
    swap(m_name_iterator, other.m_name_iterator);
}


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

size_t hash_value(const key_path& path) noexcept
{
    const auto locale = std::locale();
    size_t hash = std::hash<view>()(path.key_view());

    for (auto it = details::key_name_iterator::begin(path.key_name()); 
              it != details::key_name_iterator::end(path.key_name()); ++it) 
    {
        std::for_each(it->begin(), it->end(), [&](auto c) { boost::hash_combine(hash, std::tolower(c, locale)); });
    }
    return hash;
}

}  // namespace registry