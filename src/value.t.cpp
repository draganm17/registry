#include <array>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/value.h>

using namespace registry;


TEST(Value, Construct)
{
    // default constructor
    {
        value v;
        EXPECT_TRUE(v.type() == value_type::none);
        EXPECT_TRUE(v.data().empty());
    }

    // value::value(none_value_tag)
    {
        value v(none_value_tag{});
        EXPECT_TRUE(v.type() == value_type::none);
        EXPECT_TRUE(v.data().empty());
    }

    // value::value(sz_value_tag, string_view_type)
    {
        value v(sz_value_tag{}, TEXT("test"));
        EXPECT_TRUE(v.type() == value_type::sz);
        EXPECT_TRUE(v.to_string() == TEXT("test"));
    }

    // value::value(expand_sz_value_tag, string_view_type)
    {
        value v(expand_sz_value_tag{}, TEXT("test"));
        EXPECT_TRUE(v.type() == value_type::expand_sz);
        EXPECT_TRUE(v.to_string() == TEXT("test"));
    }

    // value::value(binary_value_tag, byte_array_view_type)
    {
        // TODO: ...
    }

    // value::value(dword_value_tag, uint32_t)
    {
        value v(dword_value_tag{}, 42);
        EXPECT_TRUE(v.type() == value_type::dword);
        EXPECT_TRUE(v.to_uint32() == 42 && v.to_uint64() == 42);
    }

    // value::value(dword_big_endian_value_tag, uint32_t);
    {
        value v(dword_big_endian_value_tag{}, 42);
        EXPECT_TRUE(v.type() == value_type::dword_big_endian);
        EXPECT_TRUE(v.to_uint32() == 42 && v.to_uint64() == 42);
    }

    // value::value(link_value_tag, string_view_type)
    {
        value v(link_value_tag{}, TEXT("test"));
        EXPECT_TRUE(v.type() == value_type::link);
        EXPECT_TRUE(v.to_string() == TEXT("test"));
    }

    // value::value(multi_sz_value_tag, const Sequence&)
    {
        const std::vector<string_type> seq_in{ TEXT("test_1"), TEXT("test_2") };
        value v(multi_sz_value_tag{}, seq_in);
        auto seq_out = v.to_strings();

        EXPECT_TRUE(v.type() == value_type::multi_sz);
        EXPECT_TRUE(std::equal(seq_out.begin(), seq_out.end(), seq_in.begin(), seq_in.end()));
    }

    // value::value(multi_sz_value_tag, InputIt, InputIt)
    {
        const auto seq_in = { TEXT("test_1"), TEXT("test_2") };
        value v(multi_sz_value_tag{}, seq_in.begin(), seq_in.end());
        auto seq_out = v.to_strings();

        EXPECT_TRUE(v.type() == value_type::multi_sz);
        EXPECT_TRUE(std::equal(seq_out.begin(), seq_out.end(), seq_in.begin(), seq_in.end()));
    }

    // value::value(multi_sz_value_tag, std::initializer_list<String>)
    {
        const auto seq_in = { TEXT("test_1"), TEXT("test_2") };
        value v(multi_sz_value_tag{}, seq_in);
        auto seq_out = v.to_strings();

        EXPECT_TRUE(v.type() == value_type::multi_sz);
        EXPECT_TRUE(std::equal(seq_out.begin(), seq_out.end(), seq_in.begin(), seq_in.end()));
    }

    // value::value(qword_value_tag, uint64_t)
    {
        value v(qword_value_tag{}, 42);
        EXPECT_TRUE(v.type() == value_type::qword);
        EXPECT_TRUE(v.to_uint64() == 42);
    }
}

TEST(Value, Assign)
{
    // assign(none_value_tag)
    {
        value v1(none_value_tag{});
        value v2(sz_value_tag{}, TEXT("test"));
        EXPECT_TRUE(v1 == v2.assign(none_value_tag{}));
    }

    // assign(sz_value_tag, string_view_type)
    {
        value v1(sz_value_tag{}, TEXT("test"));
        value v2;
        EXPECT_TRUE(v1 == v2.assign(sz_value_tag{}, TEXT("test")));
    }

    // assign(expand_sz_value_tag, string_view_type)
    {
        value v1(expand_sz_value_tag{}, TEXT("test"));
        value v2;
        EXPECT_TRUE(v1 == v2.assign(expand_sz_value_tag{}, TEXT("test")));
    }

    // assign(binary_value_tag, byte_array_view_type)
    {
        std::array<uint8_t, 2> bytes{ 4, 2 };
        value v1(binary_value_tag{}, bytes.data(), bytes.size());
        value v2;
        EXPECT_TRUE(v1 == v2.assign(binary_value_tag{}, bytes.data(), bytes.size()));
    }

    // assign(dword_value_tag, uint32_t)
    {
        value v1(dword_value_tag{}, 42);
        value v2;
        EXPECT_TRUE(v1 == v2.assign(dword_value_tag{}, 42));
    }

    // assign(dword_big_endian_value_tag, uint32_t)
    {
        value v1(dword_big_endian_value_tag{}, 42);
        value v2;
        EXPECT_TRUE(v1 == v2.assign(dword_big_endian_value_tag{}, 42));
    }

    // assign(link_value_tag, string_view_type)
    {
        value v1(link_value_tag{}, TEXT("test"));
        value v2;
        EXPECT_TRUE(v1 == v2.assign(link_value_tag{}, TEXT("test")));
    }

    // assign(multi_sz_value_tag, const Sequence&)
    {
        const std::vector<string_type> data{ TEXT("test_1"), TEXT("test_2") };

        value v1(multi_sz_value_tag{}, data);
        value v2;
        EXPECT_TRUE(v1 == v2.assign(multi_sz_value_tag{}, data));
    }

    // assign(multi_sz_value_tag, InputIt, InputIt)
    {
        const auto data = { TEXT("test_1"), TEXT("test_2") };

        value v1(multi_sz_value_tag{}, data.begin(), data.end());
        value v2;
        EXPECT_TRUE(v1 == v2.assign(multi_sz_value_tag{}, data.begin(), data.end()));
    }

    // assign(multi_sz_value_tag, std::initializer_list<String>)
    {
        value v1(multi_sz_value_tag{}, { TEXT("test_1"), TEXT("test_2") });
        value v2;
        EXPECT_TRUE(v1 == v2.assign(multi_sz_value_tag{}, { TEXT("test_1"), TEXT("test_2") }));
    }

    // assign(multi_sz_value_tag, const std::vector<string_view_type>&)
    {
        // TODO: ...
    }

    // assign(qword_value_tag, uint64_t value)
    {
        value v1(qword_value_tag{}, 42);
        value v2;
        EXPECT_TRUE(v1 == v2.assign(qword_value_tag{}, 42));
    }

    // assign(value_type type, byte_array_view_type data)
    {
        // TODO: ...
    }
}

TEST(Value, Compare)
{
    value v1;
    value v2(sz_value_tag{}, TEXT("test"));
    value v3(sz_value_tag{}, TEXT("test"));
    value v4(sz_value_tag{}, TEXT("test_2"));

    // a value is equal to itself
    EXPECT_TRUE(  v1 == v1 );  // testing operator ==
    EXPECT_TRUE(  v2 == v2 );
    EXPECT_TRUE(!(v1 != v1));  // testing operator !=
    EXPECT_TRUE(!(v2 != v2));

    // two identical value are equal
    EXPECT_TRUE(  v2 == v3 );  // testing operator ==
    EXPECT_TRUE(!(v2 != v3));  // testing operator !=

    // two different value are not equal
    EXPECT_TRUE(!(v1 == v2));  // testing operator ==
    EXPECT_TRUE(!(v3 == v4));
    EXPECT_TRUE(  v1 != v2 );  // testing operator !=
    EXPECT_TRUE(  v3 != v4 );

    // TODO: test other comparison operators
}

TEST(Value, Cast)
{
    const std::vector<uint8_t> bytes{ 4, 2 };
    const std::vector<string_type> strings{ TEXT("test1"), TEXT("test2") };

    value v1;
    EXPECT_THROW(v1.to_uint32(),  bad_value_cast);
    EXPECT_THROW(v1.to_uint64(),  bad_value_cast);
    EXPECT_THROW(v1.to_string(),  bad_value_cast);
    EXPECT_THROW(v1.to_strings(), bad_value_cast);
    EXPECT_THROW(v1.to_bytes(),   bad_value_cast);

    value v2(sz_value_tag{}, TEXT("test"));
    EXPECT_THROW(v2.to_uint32(),   bad_value_cast);
    EXPECT_THROW(v2.to_uint64(),   bad_value_cast);
    EXPECT_TRUE (v2.to_string() == TEXT("test"));
    EXPECT_THROW(v2.to_strings(),  bad_value_cast);
    EXPECT_THROW(v2.to_bytes(),    bad_value_cast);

    value v3(expand_sz_value_tag{}, TEXT("test"));
    EXPECT_THROW(v3.to_uint32(),   bad_value_cast);
    EXPECT_THROW(v3.to_uint64(),   bad_value_cast);
    EXPECT_TRUE (v3.to_string() == TEXT("test"));
    EXPECT_THROW(v3.to_strings(),  bad_value_cast);
    EXPECT_THROW(v3.to_bytes(),    bad_value_cast);

    value v4(binary_value_tag{}, bytes.data(), bytes.size());
    EXPECT_THROW(v4.to_uint32(),  bad_value_cast);
    EXPECT_THROW(v4.to_uint64(),  bad_value_cast);
    EXPECT_THROW(v4.to_string(),  bad_value_cast);
    EXPECT_THROW(v4.to_strings(), bad_value_cast);
    EXPECT_TRUE(v4.to_bytes() ==  bytes);

    value v5(dword_value_tag{}, 42);
    EXPECT_TRUE (v5.to_uint32() == 42);
    EXPECT_TRUE (v5.to_uint64() == 42);
    EXPECT_THROW(v5.to_string(),   bad_value_cast);
    EXPECT_THROW(v5.to_strings(),  bad_value_cast);
    EXPECT_THROW(v5.to_bytes(),    bad_value_cast);

    value v6(dword_big_endian_value_tag{}, 42);
    EXPECT_TRUE (v6.to_uint32() == 42);
    EXPECT_TRUE (v6.to_uint64() == 42);
    EXPECT_THROW(v6.to_string(),   bad_value_cast);
    EXPECT_THROW(v6.to_strings(),  bad_value_cast);
    EXPECT_THROW(v6.to_bytes(),    bad_value_cast);

    value v7(link_value_tag{}, TEXT("test"));
    EXPECT_THROW(v7.to_uint32(),   bad_value_cast);
    EXPECT_THROW(v7.to_uint64(),   bad_value_cast);
    EXPECT_TRUE (v7.to_string() == TEXT("test"));
    EXPECT_THROW(v7.to_strings(),  bad_value_cast);
    EXPECT_THROW(v7.to_bytes(),    bad_value_cast);

    value v8(multi_sz_value_tag{}, strings);
    EXPECT_THROW(v8.to_uint32(),       bad_value_cast);
    EXPECT_THROW(v8.to_uint64(),       bad_value_cast);
    EXPECT_THROW(v8.to_string(),       bad_value_cast);
    EXPECT_NO_THROW(v8.to_strings() == strings);
    EXPECT_THROW(v8.to_bytes(),        bad_value_cast);

    value v9(qword_value_tag{}, 42);
    EXPECT_THROW(v9.to_uint32(),   bad_value_cast);
    EXPECT_TRUE (v9.to_uint64() == 42);
    EXPECT_THROW(v9.to_string(),   bad_value_cast);
    EXPECT_THROW(v9.to_strings(),  bad_value_cast);
    EXPECT_THROW(v9.to_bytes(),    bad_value_cast);
}

TEST(Value, Swap)
{
    value v1(sz_value_tag{}, TEXT("test_1")),        v1_copy = v1;
    value v2(expand_sz_value_tag{}, TEXT("test_2")), v2_copy = v2;

    swap(v1, v2);
    EXPECT_TRUE(v1 == v2_copy && v2 == v1_copy);
}

TEST(Value, Hash)
{
    value v1, v2;
    EXPECT_TRUE(hash_value(v1) == hash_value(v2));

    value v3(sz_value_tag{}, TEXT("Test")), v4 = v3;
    EXPECT_TRUE(hash_value(v3) == hash_value(v4));
}