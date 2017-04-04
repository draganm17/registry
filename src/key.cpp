#include <algorithm>
#include <array>
#include <cassert>
#include <Windows.h>

#include <boost/algorithm/string/predicate.hpp>

#include <registry/key.h>


namespace  {

using namespace registry;

string_view_type key_id_to_string(key_id id) noexcept
{
    switch(id)
    {
        case key_id::classes_root :                return TEXT("HKEY_CLASSES_ROOT");
        case key_id::current_user :                return TEXT("HKEY_CURRENT_USER");
        case key_id::local_machine :               return TEXT("HKEY_LOCAL_MACHINE");
        case key_id::users :                       return TEXT("HKEY_USERS");
        case key_id::performance_data :            return TEXT("HKEY_PERFORMANCE_DATA");
        case key_id::performance_text :            return TEXT("HKEY_PERFORMANCE_TEXT");
        case key_id::performance_nlstext :         return TEXT("HKEY_PERFORMANCE_NLSTEXT");
        case key_id::current_config :              return TEXT("HKEY_CURRENT_CONFIG");
        case key_id::current_user_local_settings : return TEXT("HKEY_CURRENT_USER_LOCAL_SETTINGS");
        default:                                   return string_view_type{};
    };
}

key_id key_id_from_string(string_view_type str) noexcept
{
    using key_map_value_type = std::pair<string_view_type, key_id>;
    using key_map_type = std::array<key_map_value_type, 9>;

    // NOTE: keys are sorted in alphabetical order
    static const key_map_type key_map
    {
        key_map_value_type{ TEXT("HKEY_CLASSES_ROOT"),                key_id::classes_root                },
        key_map_value_type{ TEXT("HKEY_CURRENT_CONFIG"),              key_id::current_config              },
        key_map_value_type{ TEXT("HKEY_CURRENT_USER"),                key_id::current_user                },
        key_map_value_type{ TEXT("HKEY_CURRENT_USER_LOCAL_SETTINGS"), key_id::current_user_local_settings },
        key_map_value_type{ TEXT("HKEY_LOCAL_MACHINE"),               key_id::local_machine               },
        key_map_value_type{ TEXT("HKEY_PERFORMANCE_DATA"),            key_id::performance_data            },
        key_map_value_type{ TEXT("HKEY_PERFORMANCE_NLSTEXT"),         key_id::performance_nlstext         },
        key_map_value_type{ TEXT("HKEY_PERFORMANCE_TEXT"),            key_id::performance_text            },
        key_map_value_type{ TEXT("HKEY_USERS"),                       key_id::users                       }
    };

    using boost::ilexicographical_compare;
    auto it = std::lower_bound(key_map.begin(), key_map.end(), str,
                               [](auto&& lhs, auto&& rhs) { return ilexicographical_compare(lhs.first, rhs); });

    return (it != key_map.end() && (*it).first == str) ? (*it).second : key_id::none;
}

}  // anonymous namespace


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

key key::from_key_id(key_id id) { return key(key_id_to_string(id)); }

key::key(string_view_type name, registry::view view)
    : m_view(view)
    , m_name(static_cast<string_type>(name))
{ }

const string_type& key::name() const noexcept { return m_name; }

view key::view() const noexcept { return m_view; }

key key::root_key() const { return has_root_key() ? key(*begin(), view()) : key(string_view_type(), view()); }

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
    return beg_it != end_it && beg_it->data() == m_name.data() && key_id_from_string(*beg_it) != key_id::none;
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

key& key::append(string_view_type subkey)
{
    const bool add_slash = !(begin() == end() || m_name.back() == TEXT('\\') ||
                             subkey.empty() || subkey.front() == TEXT('\\'));

    m_name.reserve(m_name.size() + subkey.size() + static_cast<int>(add_slash));
    m_name.append(add_slash ? TEXT("\\") : TEXT("")).append(subkey.data(), subkey.size());
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
    for (; first != end && *first == TEXT('\\'); ++first);

    auto last = first;
    for (; last != end && *last != TEXT('\\'); ++last);

    m_value = string_view_type(first, last - first);
    return *this;
}

key::iterator key::iterator::operator++(int) noexcept { auto tmp = *this; ++*this; return tmp; }

key::iterator& key::iterator::operator--() noexcept
{
    // TODO: is begin iterator assert

    const auto rbegin = m_key_name_view.begin() - 1;

    auto last = m_value.begin() - 1;
    for (; last != rbegin && *last == TEXT('\\'); --last);

    auto first = last;
    for (; first != rbegin && *first != TEXT('\\'); --first);

    m_value = string_view_type(++first, ++last - first);
    return *this;
}

key::iterator key::iterator::operator--(int) noexcept { auto tmp = *this; --*this; return tmp; }

void key::iterator::swap(iterator& other) noexcept 
{
    m_value.swap(other.m_value);
    m_key_name_view.swap(other.m_key_name_view);
}

}  // namespace registry