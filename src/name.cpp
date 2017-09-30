#include <registry/details/common_utility.impl.h>
#include <registry/name.h>


namespace registry {

//-------------------------------------------------------------------------------------------//
//                                       class name                                          //
//-------------------------------------------------------------------------------------------//

name::name(string_type&& source) noexcept
: m_value(std::move(source))
{ }

name& name::operator=(const string_type& source)
{
    m_value = source;
    return *this;
}

name& name::operator=(string_type&& source) noexcept
{
    m_value = std::move(source);
    return *this;
}

name& name::operator=(const value_type* source)
{
    m_value = source;
    return *this;
}

name& name::operator=(string_view_type source)
{
    m_value = source;
    return *this;
}

name::operator string_type() const
{
    return m_value;
}

name::operator string_view_type() const noexcept
{
    return m_value;
}

bool name::empty() const noexcept
{
    return m_value.empty();
}

const name::value_type* name::c_str() const noexcept
{
    return m_value.c_str();
}

size_t name::size() const noexcept
{
    return m_value.size();
}

constexpr name::string_type& name::value() & noexcept
{
    return m_value;
}

constexpr const name::string_type& name::value() const & noexcept
{
    return m_value;
}

constexpr name::string_type&& name::value() && noexcept
{
    return std::move(m_value);
}

constexpr const name::string_type&& name::value() const && noexcept
{
    return std::move(m_value);
}

std::string name::string(const std::locale& loc) const
{
    // TODO: ...
    throw 0;
}

std::wstring name::wstring(const std::locale& loc) const
{
    // TODO: ...
    throw 0;
}

std::string name::u8string(const std::locale& loc) const
{
    // TODO: ...
    throw 0;
}

std::u16string name::u16string(const std::locale& loc) const
{
    // TODO: ...
    throw 0;
}

std::u32string name::u32string(const std::locale& loc) const
{
    // TODO: ...
    throw 0;
}

int name::compare(const name& name) const noexcept
{
    // TODO: ...
    throw 0;

    //const auto beg1 = m_name.begin(),       end1 = m_name.end(),
    //           beg2 = other.m_name.begin(), end2 = other.m_name.end();

    //if (std::lexicographical_compare(first1, last1, first2, last2, details::is_iless())) return -1;
    //if (std::lexicographical_compare(beg2, end2, beg1, end1, details::is_iless())) return  1;
    //return 0;
}

name& name::assign(const string_type& source)
{
    m_value.assign(source);
    return *this;
}

name& name::assign(string_type&& source) noexcept
{
    m_value.assign(std::move(source));
    return *this;
}

name& name::assign(const value_type* source)
{
    m_value.assign(source);
    return *this;
}

name& name::assign(string_view_type source)
{
    m_value.assign(source);
    return *this;
}

void name::clear() noexcept
{
    m_value.clear();
}

void name::swap(name& other) noexcept
{
    std::swap(m_value, other.m_value);
}

}  // namespace registry


namespace std {

//-------------------------------------------------------------------------------------------//
//                               class hash<registry::value>                                 //
//-------------------------------------------------------------------------------------------//

size_t hash<registry::name>::operator()(const registry::name& val) const noexcept
{
    size_t hash = 0;
    const std::locale loc;
    std::for_each(val.value().data(), val.value().data() + val.value().size(),
                  [&](auto c) { registry::details::hash_combine(hash, std::tolower(c, loc)); });
    return hash;
}

} // namespace std