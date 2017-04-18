#include <array>
#include <map>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/value_iterator.h>

using namespace registry;

TEST(ValueIterator, Construct)
{
    // default constructor
    {
        value_iterator it;
        EXPECT_TRUE(it == value_iterator());
    }

    // value_iterator::value_iterator(const key&)
    // value_iterator::value_iterator(const key&, std::error_code&)
    {
        std::error_code ec;

        value_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"));
        EXPECT_TRUE(it1 == value_iterator());

        value_iterator it2(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"), ec);
        EXPECT_TRUE(it2 == value_iterator() && !ec);

        value_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"));
        EXPECT_TRUE(it3 != value_iterator());

        value_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), ec);
        EXPECT_TRUE(it4 != value_iterator() && !ec);
    }

    // value_iterator::value_iterator(const key_handle&)
    // value_iterator::value_iterator(const key_handle&, std::error_code&)
    {
        std::error_code ec;
        const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

        value_iterator it1(key_handle(k, access_rights::query_value));
        value_iterator it2(key_handle(k, access_rights::query_value), ec);
        EXPECT_TRUE(it1 != value_iterator());
        EXPECT_TRUE(it2 != value_iterator() && !ec);
    }
}

TEST(ValueIterator, Iterate)
{
    std::error_code ec;
    const std::array<uint8_t, 2> data{ 4, 2 };
    const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

    std::map<string_type, value> expected_values;
    expected_values.emplace(TEXT("val_01"), value(none_value_tag{}));
    expected_values.emplace(TEXT("val_02"), value(sz_value_tag{}, TEXT("42")));
    expected_values.emplace(TEXT("val_03"), value(expand_sz_value_tag{}, TEXT("42")));
    expected_values.emplace(TEXT("val_04"), value(binary_value_tag{}, { data .data(), data .size()}));
    expected_values.emplace(TEXT("val_05"), value(dword_value_tag{}, 42));
    expected_values.emplace(TEXT("val_06"), value(dword_big_endian_value_tag{}, 42));
    expected_values.emplace(TEXT("val_07"), value(link_value_tag{}, TEXT("42")));
    expected_values.emplace(TEXT("val_08"), value(multi_sz_value_tag{}, { TEXT("42"), TEXT("42") }));
    expected_values.emplace(TEXT("val_09"), value(qword_value_tag{}, 42));

    // using range-based for loop
    int elements = 0;
    for (const auto& entry : value_iterator(k))
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(entry.value_name()) == entry.value());
        EXPECT_TRUE(expected_values.at(entry.value_name()) == entry.value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using operator++()
    elements = 0;
    for (auto it = value_iterator(k); it != value_iterator(); ++it)
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using operator++(int)
    elements = 0;
    for (auto it = value_iterator(k); it != value_iterator(); it++)
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using increment(error_code&)
    elements = 0;
    for (auto it = value_iterator(k); it != value_iterator(); it.increment(ec))
    {
        ++elements;
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);
}