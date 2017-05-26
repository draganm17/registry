#include <algorithm>
#include <cassert>
#include <locale>
#include <Windows.h>

#include <registry/details/common_utility.impl.h>
#include <registry/key_path.h>


namespace registry {

//------------------------------------------------------------------------------------//
//                                 class key_path                                     //
//------------------------------------------------------------------------------------//

key_path::key_path(const string_type::value_type* first, const string_type::value_type* last, view view)
    : m_view(view)
{
    do_append(first, last);
}

key_path& key_path::do_append(const string_type::value_type* first, const string_type::value_type* last, view view)
{
    const size_t input_size = last - first;
    m_name.reserve(m_name.size() + input_size + (!m_name.empty() && input_size ? 1 : 0));

    for (auto it =  details::key_name_iterator::begin(first, last);
              it != details::key_name_iterator::end  (first, last); ++it)
    {
        if (!m_name.empty()) m_name.push_back(key_path::separator);
        m_name.append(it->data(), it->size());
    }
    if (view != view::view_default) m_view = view;
    return *this;
}

key_path& key_path::do_concat(const string_type::value_type* first, const string_type::value_type* last, view view)
{
    const size_t input_size = last - first;
    m_name.reserve(m_name.size() + input_size);
    for (auto it =  details::key_name_iterator::begin(first, last);
              it != details::key_name_iterator::end  (first, last); ++it)
    {
        m_name.append(it->data(), it->size());
        m_name.push_back(key_path::separator);
    }
    if (!m_name.empty() && m_name.back() == key_path::separator) m_name.pop_back();
    if (view != view::view_default) m_view = view;
    return *this;
}

key_path& key_path::do_replace_leaf_path(const string_type::value_type* first, 
                                         const string_type::value_type* last, view view)
{
    return remove_leaf_path().do_append(first, last, view);
}

key_path key_path::from_key_id(key_id id) { return key_path(details::key_id_to_string(id)); }

key_path::key_path(view view)
    : m_view(view)
{ }

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
    auto it = details::key_name_iterator::end(m_name);
    const auto first = details::key_name_iterator::begin(m_name);

    if (it == first || --it == first) return key_path(m_view);
    return key_path(string_view_type(first->data(), it->data() - first->data() - 1), m_view);
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

    const auto beg1 = m_name.begin(),       end1 = m_name.end(),
               beg2 = other.m_name.begin(), end2 = other.m_name.end();
    if (std::lexicographical_compare(beg1, end1, beg2, end2, details::is_iless{})) return -1;
    if (std::lexicographical_compare(beg2, end2, beg1, end1, details::is_iless{})) return  1;
    return 0;
}

key_path::iterator key_path::begin() const
{
    iterator it;
    it.m_path_ptr = this;
    it.m_name_it = details::key_name_iterator::begin(m_name);
    if (it.m_name_it != details::key_name_iterator::end(m_name)) it.m_element.assign(*it.m_name_it, key_view());
    return it;
}

key_path::iterator key_path::end() const noexcept
{
    iterator it;
    it.m_path_ptr = this;
    it.m_name_it = details::key_name_iterator::end(m_name);
    return it;
}

key_path& key_path::assign(view view)
{
    m_name.clear();
    m_view = view;
    return *this;
}

key_path& key_path::remove_leaf_path()
{
    auto first = details::key_name_iterator::begin(m_name);
    auto last = details::key_name_iterator::end(m_name);

    if (first != last) {
        auto it = --last;
        m_name.resize((it != first) ? (--it, it->data() - first->data() + it->size()) : 0);
    }
    return *this;
}

void key_path::swap(key_path& other) noexcept
{
    using std::swap;
    swap(m_view, other.m_view);
    swap(m_name, other.m_name);
}

bool key_path::iterator::operator==(const iterator& rhs) const noexcept { return m_name_it == rhs.m_name_it; }

bool key_path::iterator::operator!=(const iterator& rhs) const noexcept { return !(*this == rhs); }


//------------------------------------------------------------------------------------//
//                           class key_path::iterator                                 //
//------------------------------------------------------------------------------------//

key_path::iterator::reference key_path::iterator::operator*() const noexcept
{
    assert(m_name_it != details::key_name_iterator::end(m_path_ptr->key_name()));
    return m_element;
}

key_path::iterator::pointer key_path::iterator::operator->() const noexcept
{
    assert(m_name_it != details::key_name_iterator::end(m_path_ptr->key_name()));
    return &m_element;
}

key_path::iterator& key_path::iterator::operator++()
{
    const auto name_end = details::key_name_iterator::end(m_path_ptr->key_name());
    assert(m_name_it != name_end);
    
    if (++m_name_it != name_end) m_element.assign(*m_name_it, m_path_ptr->key_view());
    return *this;
}

key_path::iterator key_path::iterator::operator++(int) { auto tmp = *this; ++*this; return tmp; }

key_path::iterator& key_path::iterator::operator--()
{
    const auto name_beg = details::key_name_iterator::begin(m_path_ptr->key_name());
    assert(m_name_it != name_beg);

    if (m_name_it-- != name_beg) m_element.assign(*m_name_it, m_path_ptr->key_view());
    return *this;
}

key_path::iterator key_path::iterator::operator--(int) { auto tmp = *this; --*this; return tmp; }

void key_path::iterator::swap(iterator& other) noexcept
{
    using std::swap;
    swap(m_element, other.m_element);
    swap(m_name_it, other.m_name_it);
    swap(m_path_ptr, other.m_path_ptr);
}


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

size_t hash_value(const key_path& path) noexcept
{
    const auto loc = std::locale();
    const auto& key_name = path.key_name();

    size_t hash = std::hash<view>()(path.key_view());
    std::for_each(key_name.begin(), key_name.end(), [&](auto c) { details::hash_combine(hash, std::tolower(c, loc)); });
    return hash;
}

}  // namespace registry