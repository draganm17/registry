#include <algorithm>
#include <cassert>
#include <numeric>
#include <Windows.h>

#include <boost/endian/arithmetic.hpp>

#include <registry/details/common_utility.impl.h>
#include <registry/value.h>


namespace registry {

//------------------------------------------------------------------------------------//
//                                 class value                                        //
//------------------------------------------------------------------------------------//

value::value(none_value_tag tag) noexcept
    : details::value_state{ value_type::none }
{ }

value::value(sz_value_tag tag, string_view_type value) { assign(tag, value); }

value::value(expand_sz_value_tag tag, string_view_type value) { assign(tag, value); }

value::value(binary_value_tag tag, byte_array_view_type value) { assign(tag, value); }

value::value(dword_value_tag tag, uint32_t value) { assign(tag, value); }

value::value(dword_big_endian_value_tag tag, uint32_t value) { assign(tag, value); }

value::value(link_value_tag tag, string_view_type value) { assign(tag, value); }

//value::value(multi_sz_value_tag tag, const std::vector<string_view_type>& value) { assign(tag, value); }

value::value(qword_value_tag tag, uint64_t value) { assign(tag, value); }

value::value(value_type type, byte_array_view_type data)
    : details::value_state{ type, { data.data(), data.data() + data.size() } }
{ }

value_type value::type() const noexcept { return m_type; }

byte_array_view_type value::data() const noexcept { return byte_array_view_type{ m_data.data(), m_data.size() }; }

uint32_t value::to_uint32() const
{
    uint8_t buf[sizeof(uint32_t)];
    memcpy(buf, m_data.data(), std::min(m_data.size(), sizeof(uint32_t)));

    using namespace boost::endian;
    switch (m_type) 
    {
        case value_type::dword:             return *reinterpret_cast<const uint32_t*>(buf);

        case value_type::dword_big_endian:  return static_cast<uint32_t>(*reinterpret_cast<const big_uint32_t*>(buf));
    }
    throw bad_value_cast();
}

uint64_t value::to_uint64() const
{
    uint8_t buf[sizeof(uint64_t)];
    memcpy(buf, m_data.data(), std::min(m_data.size(), sizeof(uint64_t)));

    using namespace boost::endian;
    switch (m_type) 
    {
        case value_type::dword:             return static_cast<uint64_t>(*reinterpret_cast<const uint32_t*>(buf));

        case value_type::dword_big_endian:  return static_cast<uint64_t>(*reinterpret_cast<const big_uint32_t*>(buf));

        case value_type::qword:             return *reinterpret_cast<const uint64_t*>(buf);
    }
    throw bad_value_cast();
}

string_type value::to_string() const
{
    if (m_type == value_type::sz || m_type == value_type::expand_sz || m_type == value_type::link)
    {
        const auto chars = m_data.size() / sizeof(string_type::value_type);
        const auto str_ptr = reinterpret_cast<const string_type::value_type*>(m_data.data());
        return chars ? string_type(str_ptr, chars - (str_ptr[chars - 1] ? 0 : 1)) : string_type{};
    }
    throw bad_value_cast();
}

std::vector<string_type> value::to_strings() const
{
    if (m_type == value_type::multi_sz)
    {
        std::vector<string_type> result;
        const auto chars = m_data.size() / sizeof(string_type::value_type);
        const auto str_ptr = reinterpret_cast<const string_type::value_type*>(m_data.data());
        
        auto first = str_ptr, last = str_ptr, end = &str_ptr[chars];
        while (last != end) {
            if (!(*last++) || last == end) {
                if (!(last == end && last - first == 1 && *first == TEXT('\0'))) {
                    result.emplace_back(first, last - (last == end ? 0 : 1));
                }
                first = last;
            }
        }
        return result;
    }
    throw bad_value_cast();
}

byte_array_type value::to_byte_array() const
{
    return m_type == value_type::binary ? m_data : throw bad_value_cast();
}

value& value::assign(none_value_tag) noexcept
{
    m_data.clear();
    m_type = value_type::none;

    return *this;
}

value& value::assign(sz_value_tag, string_view_type value)
{
    static constexpr auto null_teminator = TEXT('\0');
    const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
    const auto data_size = value.size() * sizeof(string_view_type::value_type);

    m_data.resize(data_size + sizeof(null_teminator));
    m_type = value_type::sz;

    memcpy(m_data.data(), data_ptr, data_size);
    memcpy(m_data.data() + data_size, &null_teminator, sizeof(null_teminator));

    return *this;
}

value& value::assign(expand_sz_value_tag, string_view_type value)
{
    static constexpr auto null_teminator = TEXT('\0');
    const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
    const auto data_size = value.size() * sizeof(string_view_type::value_type);

    m_data.resize(data_size + sizeof(null_teminator));
    m_type = value_type::expand_sz;

    memcpy(m_data.data(), data_ptr, data_size);
    memcpy(m_data.data() + data_size, &null_teminator, sizeof(null_teminator));

    return *this;
}

value& value::assign(binary_value_tag, byte_array_view_type value)
{
    m_data.resize(value.size());
    m_type = value_type::binary;
    memcpy(m_data.data(), value.data(), value.size());

    return *this;
}

value& value::assign(dword_value_tag, uint32_t value)
{
    m_data.resize(sizeof(uint32_t));
    m_type = value_type::dword;
    memcpy(m_data.data(), &value, sizeof(uint32_t));

    return *this;
}

value& value::assign(dword_big_endian_value_tag, uint32_t value)
{
    m_data.resize(sizeof(uint32_t));
    m_type = value_type::dword_big_endian;

    boost::endian::big_uint32_t value_copy = value;
    memcpy(m_data.data(), value_copy.data(), sizeof(uint32_t));

    return *this;
}

value& value::assign(link_value_tag, string_view_type value)
{
    static constexpr auto null_teminator = TEXT('\0');
    const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
    const auto data_size = value.size() * sizeof(string_view_type::value_type);

    m_data.resize(data_size + sizeof(null_teminator));
    m_type = value_type::link;

    memcpy(m_data.data(), data_ptr, data_size);
    memcpy(m_data.data() + data_size, &null_teminator, sizeof(null_teminator));

    return *this;
}

value& value::assign(multi_sz_value_tag, const std::vector<string_view_type>& value)
{
    static constexpr auto null_teminator = TEXT('\0');

    const auto buffer_size = std::accumulate(value.begin(), value.end(), size_t(0), [](size_t sz, const auto& value) {
        return sz + value.size() * sizeof(string_type::value_type) + sizeof(null_teminator);
    }) + sizeof(null_teminator);

    m_data.resize(buffer_size);
    m_type = value_type::multi_sz;

    size_t offset = 0;
    std::for_each(value.begin(), value.end(), [&](const auto& value) noexcept
    {
        const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
        const auto data_size = value.size() * sizeof(string_view_type::value_type);

        memcpy(m_data.data() + offset, data_ptr, data_size);
        memcpy(m_data.data() + data_size + offset, &null_teminator, sizeof(null_teminator));
        offset += data_size + sizeof(null_teminator);
    });
    memcpy(m_data.data() + offset, &null_teminator, sizeof(null_teminator));

    return *this;
}
/*
value& value::assign_impl(multi_sz_value_tag, const std::function<bool(string_view_type&)>& enumerator)
{
    string_view_type value;
    auto enumerator_copy = enumerator;
    size_t offset = 0, buffer_size = 0;
    static constexpr auto null_teminator = TEXT('\0');

    while (enumerator(value)) {
        buffer_size += value.size() * sizeof(string_type::value_type) + sizeof(null_teminator);
    }

    m_data.resize(buffer_size);
    m_type = value_type::multi_sz;
    while (enumerator_copy(value))
    {
        const auto data_ptr = reinterpret_cast<const uint8_t*>(value.data());
        const auto data_size = value.size() * sizeof(string_view_type::value_type);

        memcpy(m_data.data() + offset, data_ptr, data_size);
        memcpy(m_data.data() + data_size + offset, &null_teminator, sizeof(null_teminator));
        offset += data_size + sizeof(null_teminator);
    }
    memcpy(m_data.data() + offset, &null_teminator, sizeof(null_teminator));

    return *this;
}*/

value& value::assign(qword_value_tag, uint64_t value)
{
    m_data.resize(sizeof(uint64_t));
    m_type = value_type::qword;
    memcpy(m_data.data(), &value, sizeof(uint64_t));

    return *this;
}

void value::swap(value& other) noexcept
{
    using std::swap;
    swap(m_type, other.m_type);
    swap(m_data, other.m_data);
}


//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

size_t hash_value(const value& value) noexcept
{
    const size_t sz = value.data().size();
    const unsigned char* ptr = value.data().data();

    size_t hash = std::hash<value_type>()(value.type());
    for (auto i = sz % sizeof(long); i; --i, ++ptr) details::hash_combine(hash, *ptr);
    for (auto i = sz / sizeof(long); i; --i, ptr += sizeof(long)) details::hash_combine(hash, *(const long*)ptr);

    return hash;
}

}  // namespace registry