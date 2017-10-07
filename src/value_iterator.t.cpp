#include <array>
#include <map>
#include <functional>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/exception.h>
#include <registry/key.h>
#include <registry/name.h>
#include <registry/value_iterator.h>

using namespace registry;


namespace {

void test_iteration(const std::function<value_iterator()>& get_iterator)
{
    std::error_code ec;
    const std::array<uint8_t, 2> bytes{ 4, 2 };

    std::map<name, value> expected_values;
    expected_values.emplace("val_01", value(none_value_tag()));
    expected_values.emplace("val_02", value(sz_value_tag{}, "42"));
    expected_values.emplace("val_03", value(expand_sz_value_tag{}, "42"));
    expected_values.emplace("val_04", value(binary_value_tag(), bytes.data(), bytes.size()));
    expected_values.emplace("val_05", value(dword_value_tag{}, 42));
    expected_values.emplace("val_06", value(dword_big_endian_value_tag{}, 42));
    expected_values.emplace("val_07", value(link_value_tag{}, "42"));
    expected_values.emplace("val_08", value(multi_sz_value_tag{}, { "42", "42" }));
    expected_values.emplace("val_09", value(qword_value_tag{}, 42));

    // using range-based for loop
    int elements = 0;
    for (const auto& entry : get_iterator())
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(entry.value_name()) == entry.read_value());
        EXPECT_TRUE(expected_values.at(entry.value_name()) == entry.read_value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using operator++()
    elements = 0;
    for (auto it = get_iterator(); it != value_iterator(); ++it)
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->read_value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->read_value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using operator++(int)
    elements = 0;
    for (auto it = get_iterator(); it != value_iterator(); it++)
    {
        ++elements;
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->read_value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->read_value(ec) && !ec);
    }
    EXPECT_TRUE(elements == 9);

    // using increment(error_code&)
    elements = 0;
    for (auto it = get_iterator(); it != value_iterator(); it.increment(ec))
    {
        ++elements;
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->read_value());
        EXPECT_TRUE(expected_values.at(it->value_name()) == it->read_value(ec) && !ec);
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

    // value_iterator::value_iterator(const key&)
    // value_iterator::value_iterator(const key&, std::error_code&)
    {
        std::error_code ec;
        const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";

        // right permissions
        const key k1(open_only_tag{}, p, access_rights::query_value);
        value_iterator it1a(k1);
        EXPECT_TRUE(it1a != value_iterator());
        //
        value_iterator it1b(k1, ec);
        EXPECT_TRUE(!ec && it1b != value_iterator());

        // wrong permissions
        const key k2(open_only_tag{}, p, access_rights::set_value);
        EXPECT_THROW(value_iterator it2a(k2), registry_error);
        //
        value_iterator it2b(k2, ec);
        EXPECT_TRUE(ec && it2b == value_iterator());
    }

    // value_iterator::value_iterator(const key_path&)
    // value_iterator::value_iterator(const key_path&, std::error_code&)
    {
        std::error_code ec;

        value_iterator it1("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent");
        EXPECT_TRUE(it1 == value_iterator());

        value_iterator it2("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent", ec);
        EXPECT_TRUE(it2 == value_iterator() && !ec);

        value_iterator it3("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");
        EXPECT_TRUE(it3 != value_iterator());

        value_iterator it4("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read", ec);
        EXPECT_TRUE(it4 != value_iterator() && !ec);
    }
}

TEST(ValueIterator, ConstructFromKeyAndIterate)
{
    const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";
    const key k(open_only_tag{}, p, access_rights::query_value);

    test_iteration([&]() { return value_iterator(k); });
}

TEST(ValueIterator, ConstructFromPathAndIterate)
{
    const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";

    test_iteration([&]() { return value_iterator(p); });
}