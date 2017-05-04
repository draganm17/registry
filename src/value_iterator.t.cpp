#include <array>
#include <map>
#include <functional>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key.h>
#include <registry/value_iterator.h>

using namespace registry;


namespace {

void test_iteration(const std::function<value_iterator()>& get_iterator)
{
    std::error_code ec;
    const std::array<uint8_t, 2> data{ 4, 2 };

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
    for (const auto& entry : get_iterator())
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(entry.value_name()) == entry.value());
        EXPECT_TRUE(expected_values.at(entry.value_name()) == entry.value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using operator++()
    elements = 0;
    for (auto it = get_iterator(); it != value_iterator(); ++it)
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using operator++(int)
    elements = 0;
    for (auto it = get_iterator(); it != value_iterator(); it++)
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using increment(error_code&)
    elements = 0;
    for (auto it = get_iterator(); it != value_iterator(); it.increment(ec))
    {
        ++elements;
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);
}

}


TEST(ValueIterator, Construct)
{
    // default constructor
    {
        value_iterator it;
        EXPECT_TRUE(it == value_iterator());
    }

    // value_iterator::value_iterator(const key_entry&)
    // value_iterator::value_iterator(const key_entry&, std::error_code&)
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
}

TEST(ValueIterator, ObtainFromPathAndIterate)
{
    const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

    test_iteration([&]() { return value_iterator(p); });
}

TEST(ValueIterator, ObtainFromKeyAndIterate)
{
    const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");
    const key k(open_only_tag{}, p, access_rights::query_value);

    test_iteration([&]() { return k.get_value_iterator(); });
}