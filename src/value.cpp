#include <algorithm>
#include <numeric>

#include <boost/endian/arithmetic.hpp> // TODO: get rid of that dependency

#include <registry/details/common_utility.impl.h>
#include <registry/value.h>


namespace registry {

//-------------------------------------------------------------------------------------------//
//                                       class value                                         //
//-------------------------------------------------------------------------------------------//

value& value::do_assign(value_type type, const void* data, size_t size)
{
    m_data.resize(size);
    m_type = type;
    memcpy(m_data.data(), data, size);
    return *this;
}

value& value::do_assign(value_type type, std::basic_string_view<name::value_type> val)
{
    // TODO: can be not null-terminated

    return do_assign(type, val.data(), (val.size() + 1) * sizeof(wchar_t));
}

value& value::do_assign(value_type type, std::vector<name>&& val)
{
    size_t buf_offs = 0;
    size_t buf_size = sizeof(wchar_t) + 
                      std::accumulate(val.begin(), val.end(), 0, [](auto sz, auto&& el)
                                      { return sz + (el.size() + 1) * sizeof(wchar_t); });

    m_data.resize(buf_size);
    for (const auto& el : val)
    {
        size_t copy_bytes = (el.size() + 1) * sizeof(wchar_t);
        memcpy(m_data.data() + buf_offs, el.data(), copy_bytes);
        buf_offs += copy_bytes;
    }

    m_type = type;
    constexpr wchar_t null_char{};
    memcpy(m_data.data() + buf_offs, &null_char, sizeof(null_char));

    return *this;
}

value::value(nullptr_t)
: value()
{ }

value::value(none_value_tag)
: value()
{ }

value::value(sz_value_tag, const name& val)
{
    assign(sz_value_tag(), val);
}

value::value(expand_sz_value_tag, const name& val)
{
    assign(expand_sz_value_tag(), val);
}

value::value(binary_value_tag, const void* data, size_t size)
{
    assign(binary_value_tag(), data, size);
}

value::value(dword_value_tag, uint32_t val)
{
    assign(dword_value_tag(), val);
}

value::value(dword_big_endian_value_tag, uint32_t val)
{
    assign(dword_big_endian_value_tag(), val);
}

value::value(link_value_tag, const name& val)
{
    assign(link_value_tag(), val);
}

value::value(qword_value_tag, uint64_t val)
{
    assign(qword_value_tag(), val);
}

value::value(value_type type, const void* data, size_t size)
{
    assign(type, data, size);
}

value& value::operator=(nullptr_t)
{
    return assign(nullptr);
}

value_type value::type() const noexcept
{
    return m_type;
}

const void* value::data() const noexcept
{
    return m_data.size() ? m_data.data() : nullptr;
}

size_t value::size() const noexcept
{
    return m_data.size();
}

uint32_t value::to_uint32() const
{
    using namespace boost::endian;
    unsigned char buf[sizeof(uint32_t)];
    memcpy(buf, m_data.data(), std::min(m_data.size(), sizeof(uint32_t)));

    switch (m_type)
    {
        case value_type::dword:
             return *reinterpret_cast<const uint32_t*>(buf);

        case value_type::dword_big_endian:
             return static_cast<uint32_t>(*reinterpret_cast<const big_uint32_t*>(buf));
    }
    throw bad_value_cast();
}

uint64_t value::to_uint64() const
{
    using namespace boost::endian;
    unsigned char buf[sizeof(uint64_t)];
    memcpy(buf, m_data.data(), std::min(m_data.size(), sizeof(uint64_t)));

    switch (m_type) 
    {
        case value_type::dword:
             return static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(buf));

        case value_type::dword_big_endian:
             return static_cast<uint64_t>(*reinterpret_cast<const big_uint32_t*>(buf));

        case value_type::qword:
             return *reinterpret_cast<const uint64_t*>(buf);
    }
    throw bad_value_cast();
}

std::string value::to_string(const std::locale& loc) const
{
    //using namespace denc;
    if (m_type == value_type::sz || m_type == value_type::expand_sz || m_type == value_type::link)
    {
        const size_t size = m_data.size() / sizeof(wchar_t);
        const wchar_t* data = reinterpret_cast<const wchar_t*>(m_data.data());
        const bool is_null_terminated = size && !data[size - 1];

        // Convert the string to the system narrow encoding.
        //return codec<native_wide, native_narrow>(loc)
        //       (data, data + size - static_cast<int>(is_null_terminated), use_basic_string<char>());

        // TODO: ...
        throw 0;
    }
    throw bad_value_cast();
}

std::wstring value::to_wstring() const
{
    if (m_type == value_type::sz || m_type == value_type::expand_sz || m_type == value_type::link)
    {
        const size_t size             = m_data.size() / sizeof(wchar_t);
        const wchar_t* data           = reinterpret_cast<const wchar_t*>(m_data.data());
        const bool is_null_terminated = size && !data[size - 1];

        // The string should already be in Unicode. No encoding is done.
        return std::wstring(data, data + size - static_cast<int>(is_null_terminated));
    }
    throw bad_value_cast();
}

std::vector<std::string> value::to_strings(const std::locale& loc) const
{
    if (m_type == value_type::multi_sz)
    {
        // TODO: ...
    }
    throw bad_value_cast();
}

std::vector<std::wstring> value::to_wstrings() const
{
    if (m_type == value_type::multi_sz)
    {
        // NOTE: no encoding is done.

        std::vector<std::wstring> result;
        const auto size = m_data.size() / sizeof(wchar_t);
        const auto data = reinterpret_cast<const wchar_t*>(m_data.data());
        
        auto first = data, last = data, end = data + size;
        while (last != end) {
            if (!(*last++) || last == end) {
                if (!(last == end && last - first == 1 && !*first)) {
                    result.emplace_back(first, last - (last == end ? 0 : 1));
                }
                first = last;
            }
        }
        return result;
    }
    throw bad_value_cast();
}

value& value::assign(nullptr_t)
{
    return do_assign(value_type::none, nullptr, 0);
}

value& value::assign(none_value_tag)
{
    return do_assign(value_type::none, nullptr, 0);
}

value& value::assign(sz_value_tag, const name& val)
{
    return do_assign(value_type::sz, val);
}

value& value::assign(expand_sz_value_tag, const name& val)
{
    return do_assign(value_type::expand_sz, val);
}

value& value::assign(binary_value_tag, const void* data, size_t size)
{
    return do_assign(value_type::binary, data, size);
}

value& value::assign(dword_value_tag tag, uint32_t val)
{
    return do_assign(value_type::dword, &val, sizeof(val));
}

value& value::assign(dword_big_endian_value_tag tag, uint32_t val)
{
    boost::endian::big_uint32_t val_copy = val;
    return do_assign(value_type::dword_big_endian, val_copy.data(), sizeof(val));
}

value& value::assign(link_value_tag, const name& val)
{
    return do_assign(value_type::link, val);
}

value& value::assign(qword_value_tag tag, uint64_t val)
{
    return do_assign(value_type::qword, &val, sizeof(val));
}

value& value::assign(value_type type, const void* data, size_t size)
{
    return do_assign(type, data, size);
}

void value::swap(value& other) noexcept
{
    using std::swap;
    swap(m_type, other.m_type);
    swap(m_data, other.m_data);
}

}  // namespace registry


namespace std {

//-------------------------------------------------------------------------------------------//
//                               class hash<registry::value>                                 //
//-------------------------------------------------------------------------------------------//

size_t hash<registry::value>::operator()(const registry::value& val) const noexcept
{
    const size_t sz = val.size();
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>(val.data());
    size_t hash = std::hash<registry::value_type>()(val.type());

    for (auto i = sz % sizeof(long); i; --i, ++ptr)
        registry::details::hash_combine(hash, *ptr);

    for (auto i = sz / sizeof(long); i; --i, ptr += sizeof(long))
        registry::details::hash_combine(hash, *(const long*)ptr);

    return hash;
}

} // namespace std