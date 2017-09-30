#include <algorithm>
#include <cassert>

#include <registry/details/common_utility.impl.h>
#include <registry/key_path.h>


namespace
{
    using namespace registry;

    void remove_redundant_separators(name::string_type &str) noexcept
    {
        // TODO:
        throw 0;
    }
}


namespace registry {

//-------------------------------------------------------------------------------------------//
//                                     class key_path                                        //
//-------------------------------------------------------------------------------------------//

key_path key_path::from_key_id(key_id id)
{
    return key_path(details::key_id_to_string(id));
}

key_path::key_path(view view)
: m_view(view)
{ }

key_path::key_path(name&& name, view view)
: m_view(view)
, m_name(std::move(name))
{
    remove_redundant_separators(m_name.value());
}

key_path& key_path::operator=(name&& name)
{
    return assign(std::move(name));
}

const name& key_path::key_name() const noexcept
{
    return m_name;
}

view key_path::key_view() const noexcept
{
    return m_view;
}

key_path key_path::root_path() const
{
    return root_key_id() != key_id::unknown 
           ? key_path(*details::key_name_iterator::begin(m_name), m_view) : key_path(m_view);
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

    return (it == first || --it == first) ? key_path(m_view) 
                                          : key_path(first->data(), it->data(), m_view);
}

key_path key_path::relative_path() const
{
    if (!has_root_path()) return *this;
    const auto it = ++details::key_name_iterator::begin(m_name);
    return it == details::key_name_iterator::end(m_name) ? key_path(m_view)
                                                         : key_path(it->data(), m_view);
}

bool key_path::has_root_path() const noexcept
{
    return root_key_id() != key_id::unknown;
}

bool key_path::has_leaf_path() const noexcept 
{
    return details::key_name_iterator::begin(m_name) != details::key_name_iterator::end(m_name);
}

bool key_path::has_parent_path() const noexcept
{
    return std::distance(details::key_name_iterator::begin(m_name),
                         details::key_name_iterator::end(m_name)) > 1;
}

bool key_path::has_relative_path() const noexcept 
{
    const bool has_root = has_root_path();
    auto first = details::key_name_iterator::begin(m_name);
    const auto last = details::key_name_iterator::end(m_name);
    return (has_root && ++first != last) || (!has_root && first != last);
}

bool key_path::is_absolute() const noexcept
{
    return has_root_path();
}

bool key_path::is_relative() const noexcept
{
    return !is_absolute();
}

int key_path::compare(const key_path& other) const noexcept
{
    // TODO: compare element-wise
    throw 0;

    //return (m_view != other.m_view) ? m_view < other.m_view ? -1 : 1
    //                                : m_name.compare(other.m_name);
}

key_path::iterator key_path::begin() const
{
    iterator it;
    it.m_path_ptr = this;
    it.m_name_it = details::key_name_iterator::begin(m_name);

    if (it.m_name_it != details::key_name_iterator::end(m_name)) {
        it.m_element.assign(*it.m_name_it, key_view());
    }
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
    m_view = view;
    m_name.clear();
    return *this;
}

key_path& key_path::assign(name&& name, view view)
{
    m_view = view;
    m_name = std::move(name);
    remove_redundant_separators(m_name.value());
    return *this;
}

key_path& key_path::append(const key_path& path)
{
    const bool sp = !m_name.empty() && !path.m_name.empty();
    m_name.value().reserve(m_name.size() + path.m_name.size() + static_cast<int>(sp));

    if (sp) m_name.value() += separator;
    m_name.value() += path.m_name.value();
    m_view = path.m_view != view::view_default ? path.m_view : m_view;

    return *this;
}

key_path& key_path::concat(const key_path& path)
{
    m_name.value().reserve(m_name.size() + path.m_name.size());
    m_name.value() += path.m_name.value();
    m_view = path.m_view != view::view_default ? path.m_view : m_view;

    return *this;
}

key_path& key_path::remove_leaf_path()
{
    auto first = details::key_name_iterator::begin(m_name);
    auto last  = details::key_name_iterator::end(m_name);

    if (first != last) {
        auto it = --last;
        m_name.value().resize((it != first) ? (--it, it->data() - first->data() + it->size()) : 0);
    }
    return *this;
}

key_path& key_path::replace_leaf_path(const key_path& path)
{
    return remove_leaf_path().append(path);
}

void key_path::swap(key_path& other) noexcept
{
    using std::swap;
    swap(m_view, other.m_view);
    swap(m_name, other.m_name);
}

bool key_path::iterator::operator==(const iterator& rhs) const noexcept
{
    return m_name_it == rhs.m_name_it;
}

bool key_path::iterator::operator!=(const iterator& rhs) const noexcept
{
    return !(*this == rhs);
}


//-------------------------------------------------------------------------------------------//
//                                class key_path::iterator                                   //
//-------------------------------------------------------------------------------------------//

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

key_path::iterator key_path::iterator::operator++(int)
{
    auto tmp = *this; ++*this; return tmp;
}

key_path::iterator& key_path::iterator::operator--()
{
    const auto name_beg = details::key_name_iterator::begin(m_path_ptr->key_name());
    assert(m_name_it != name_beg);

    if (m_name_it-- != name_beg) m_element.assign(*m_name_it, m_path_ptr->key_view());
    return *this;
}

key_path::iterator key_path::iterator::operator--(int)
{
    auto tmp = *this; --*this; return tmp;
}

void key_path::iterator::swap(iterator& other) noexcept
{
    using std::swap;
    swap(m_element, other.m_element);
    swap(m_name_it, other.m_name_it);
    swap(m_path_ptr, other.m_path_ptr);
}

} // namespace registry


namespace std {

//-------------------------------------------------------------------------------------------//
//                             class hash<registry::key_path>                                //
//-------------------------------------------------------------------------------------------//

size_t hash<registry::key_path>::operator()(const registry::key_path& val) const noexcept
{
    size_t hash = std::hash<registry::view>()(val.key_view());
    registry::details::hash_combine(hash, std::hash<registry::name>()(val.key_name()));
    return hash;
}

} // namespace std