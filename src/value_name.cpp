#include <registry/value_name.h>


namespace registry {

//-------------------------------------------------------------------------------------------//
//                                    class value_name                                       //
//-------------------------------------------------------------------------------------------//

value_name::value_name(const std::wstring& source)
: m_name(source)
{ }

value_name::value_name(std::wstring&& source) noexcept
: m_name(std::move(source))
{ }

value_name& value_name::operator=(const std::wstring& source)
{
    m_name = source;
    return *this;
}

value_name& value_name::operator=(std::wstring&& source) noexcept
{
    m_name = std::move(source);
    return *this;
}

bool value_name::empty() const noexcept
{
    return m_name.empty();
}

const wchar_t* value_name::c_str() const noexcept
{
    return m_name.c_str();
}

size_t value_name::size() const noexcept
{
    return m_name.size();
}

const std::wstring& value_name::get() const noexcept
{
    return m_name;
}

std::string value_name::string(const std::locale& loc)
{
    // TODO: ...
    throw 0;
}

std::wstring value_name::wstring()
{
    return m_name;
}

value_name::operator std::wstring() const
{
    return m_name;
}

int value_name::compare(const value_name& name) const noexcept
{
    // TODO: ...
    throw 0;
}
        
int value_name::compare(const std::wstring& str) const
{
    // TODO: ...
    throw 0;
}
        
int value_name::compare(std::wstring_view str) const
{
    // TODO: ...
    throw 0;
}
        
int value_name::compare(const wchar_t* s) const
{
    // TODO: ...
    throw 0;
}

value_name& value_name::assign(const std::wstring& source)
{
    m_name.assign(source);
    return *this;
}

value_name& value_name::assign(std::wstring&& source) noexcept
{
    m_name.assign(std::move(source));
    return *this;
}

void value_name::clear() noexcept
{
    m_name.clear();
}

void value_name::swap(value_name& other) noexcept
{
    std::swap(m_name, other.m_name);
}

}  // namespace registry


namespace std {

//-------------------------------------------------------------------------------------------//
//                               class hash<registry::value>                                 //
//-------------------------------------------------------------------------------------------//

size_t hash<registry::value_name>::operator()(const registry::value_name& val) const noexcept
{
    // TODO: ...
    throw 0;
}

} // namespace std