#include <algorithm>
#include <cassert>
#include <locale>
#include <Windows.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>

#include <registry/details/utils.impl.h>
#include <registry/key.h>


namespace
{
using namespace registry;

class key_pool
{
private:
    key_pool() = default;

public:
    static const key_pool& instance() { static const key_pool pool; return pool; }

public:
    const key& get(key_id id) const noexcept
    {
        switch (id) {
            case key_id::classes_root:                 return m_classes_root;
            case key_id::current_user:                 return m_current_user;
            case key_id::local_machine:                return m_local_machine;
            case key_id::users:                        return m_users;
            case key_id::performance_data:             return m_performance_data;
            case key_id::performance_text:             return m_performance_text;
            case key_id::performance_nlstext:          return m_performance_nlstext;
            case key_id::current_config:               return m_current_config;
            case key_id::current_user_local_settings:  return m_current_user_local_settings;
        }
        return m_unknown;
    }

private:
    const key m_classes_root =                key(details::key_id_to_string(key_id::classes_root));
    const key m_current_user =                key(details::key_id_to_string(key_id::current_user));
    const key m_local_machine =               key(details::key_id_to_string(key_id::local_machine));
    const key m_users =                       key(details::key_id_to_string(key_id::users));
    const key m_performance_data =            key(details::key_id_to_string(key_id::performance_data));
    const key m_performance_text =            key(details::key_id_to_string(key_id::performance_text));
    const key m_performance_nlstext =         key(details::key_id_to_string(key_id::performance_nlstext));
    const key m_current_config =              key(details::key_id_to_string(key_id::current_config));
    const key m_current_user_local_settings = key(details::key_id_to_string(key_id::current_user_local_settings));
    const key m_unknown =                     key();
};

}


namespace registry {

//------------------------------------------------------------------------------------//
//                                   class key                                        //
//------------------------------------------------------------------------------------//

const view key::default_view =
#if defined(_WIN64)
    view::view_64bit;
#elif defined(_WIN32)
    view::view_32bit;
#endif

key& key::append_impl(string_view_type subkey)
{
    const bool add_slash = !(begin() == end() || m_name.back() == separator ||
                             subkey.empty() || subkey.front() == separator);


    m_name.reserve(m_name.size() + subkey.size() + static_cast<int>(add_slash));
    m_name.append(add_slash ? TEXT("\\") : TEXT("")).append(subkey.data(), subkey.size());
    return *this;
}

key key::from_key_id(key_id id) { return key_pool::instance().get(id); }

key::key(string_view_type name, registry::view view)
    : m_view(view)
    , m_name(static_cast<string_type>(name))
{ }

const string_type& key::name() const noexcept { return m_name; }

view key::view() const noexcept { return m_view; }

key key::root_key() const { return has_root_key() ? key(*begin(), view()) : key(string_view_type(), view()); }

key_id key::root_key_id() const { return details::key_id_from_string(*begin()); }

key key::leaf_key() const { return has_leaf_key() ? key(*--end(), view()) : key(string_view_type(), view()); }

key key::parent_key() const
{
    auto first = begin(), last = end();
    key key(string_view_type(), view());

    if (first != last && first != --last) {
        for (; first != last; ++first) key.append(*first);
    }
    return key;
}

bool key::has_root_key() const noexcept { return begin() != end(); }

bool key::has_leaf_key() const noexcept { return begin() != end(); }

bool key::has_parent_key() const noexcept 
{
    auto beg_it = begin(), end_it = end();
    return beg_it != end_it && ++beg_it != end_it;
}

bool key::is_absolute() const noexcept 
{
    const auto beg_it = begin(), end_it = end();
    return beg_it != end_it && beg_it->data() == m_name.data() && 
           details::key_id_from_string(*beg_it) != key_id::unknown;
}

bool key::is_relative() const noexcept { return !is_absolute(); }

int key::compare(const key& other) const noexcept
{
    if (view() != other.view()) {
        return view() < other.view() ? -1 : 1;
    }

    iterator beg_1 = begin(), end_1 = end();
    iterator beg_2 = other.begin(), end_2 = other.end();
    for (; beg_1 != end_1 && beg_2 != end_2; ++beg_1, ++beg_2) {
        if (boost::ilexicographical_compare(*beg_1, *beg_2)) return -1;
        if (boost::ilexicographical_compare(*beg_2, *beg_1)) return  1;
    }
    return int(beg_2 == end_2) - int(beg_1 == end_1);
}

key::iterator key::begin() const noexcept
{
    iterator it;
    it.m_value = string_view_type(m_name.data(), 0);
    it.m_key_name_view = string_view_type(m_name.data(), m_name.size());
    return ++it;
}

key::iterator key::end() const noexcept
{
    iterator it;
    it.m_value = string_view_type(m_name.data() + m_name.size(), 0);
    it.m_key_name_view = string_view_type(m_name.data(), m_name.size());
    return it;
}

key& key::assign(string_view_type name, registry::view view)
{
    m_name.assign(name.data(), name.size());
    m_view = view;
    return *this;
}

key& key::append(const key& subkey)
{
    m_view = subkey.view();
    for (auto it = subkey.begin(); it != subkey.end(); ++it) append(*it);
    return *this;
}

key& key::concat(string_view_type subkey)
{
    m_name.append(subkey.data(), subkey.size());
    return *this;
}

key& key::remove_leaf()
{
    assert(has_leaf_key());

    auto it = --end();
    m_name.resize((it != begin()) ? (--it, it->data() - m_name.data() + it->size()) : 0);
    return *this;
}

key& key::replace_leaf(string_view_type replacement)
{
    assert(has_leaf_key());
    return remove_leaf().append(replacement);
}

void key::swap(key& other) noexcept
{
    using std::swap;
    swap(m_view, other.m_view);
    swap(m_name, other.m_name);
}

bool key::iterator::operator==(const iterator& rhs) const noexcept
{
    return m_value.data() == rhs.m_value.data() && m_value.size() == rhs.m_value.size();
}

bool key::iterator::operator!=(const iterator& rhs) const noexcept { return !(*this == rhs); }


//------------------------------------------------------------------------------------//
//                             class key::iterator                                    //
//------------------------------------------------------------------------------------//

key::iterator::reference key::iterator::operator*() const noexcept
{
    // TODO: is end iterator assert

    return m_value;
}

key::iterator::pointer key::iterator::operator->() const noexcept
{
    // TODO: is end iterator assert

    return &m_value;
}

key::iterator& key::iterator::operator++() noexcept
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

key::iterator key::iterator::operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }

key::iterator& key::iterator::operator--() noexcept
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

key::iterator key::iterator::operator--(int) noexcept { auto tmp = *this; --*this; return tmp; }

void key::iterator::swap(iterator& other) noexcept 
{
    m_value.swap(other.m_value);
    m_key_name_view.swap(other.m_key_name_view);
}


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

size_t hash_value(const key& key) noexcept
{
    const auto locale = std::locale();
    size_t hash = std::hash<view>()(key.view());
    for (auto it = key.begin(); it != key.end(); ++it) {
        std::for_each(it->begin(), it->end(), [&](auto c) { boost::hash_combine(hash, std::tolower(c, locale)); });
    }
    return hash;
}

}  // namespace registry