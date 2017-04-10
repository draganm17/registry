#include <set>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key_iterator.h>

using namespace registry;

TEST(KeyIterator, Construct)
{
    // default constructor
    {
        key_iterator it;
        assert(it == key_iterator());
    }

    // key_iterator::key_iterator(const key&)
    // key_iterator::key_iterator(const key&, std::error_code&)
    {
        std::error_code ec;

        key_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"));
        key_iterator it2(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"), ec);
        EXPECT_TRUE(it1 == key_iterator());
        EXPECT_TRUE(it2 == key_iterator() && !ec);

        key_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"));
        key_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), ec);
        EXPECT_TRUE(it3 != key_iterator());
        EXPECT_TRUE(it4 != key_iterator() && !ec);
    }

    // key_iterator::key_iterator(const key_handle&)
    // key_iterator::key_iterator(const key_handle&, std::error_code&)
    {
        // TODO: ...
    }
}

TEST(KeyIterator, Iterate)
{
    std::error_code ec;
    const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

    std::set<key> expected_keys;
    expected_keys.emplace(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read\\key_1_deep_0"));
    expected_keys.emplace(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read\\key_2_deep_0"));
    expected_keys.emplace(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read\\key_3_deep_0"));

    // using range-based for loop
    int elements = 0;
    for (const auto& entry : key_iterator(k))
    {
        ++elements;
        EXPECT_TRUE(expected_keys.find(entry.key()) != expected_keys.end());
    }
    EXPECT_TRUE(elements == 3);

    // using operator++()
    elements = 0;
    for (auto it = key_iterator(k); it != key_iterator(); ++it)
    {
        ++elements;
        EXPECT_TRUE(expected_keys.find(it->key()) != expected_keys.end());
    }
    EXPECT_TRUE(elements == 3);

    // using operator++(int)
    elements = 0;
    for (auto it = key_iterator(k); it != key_iterator(); it++)
    {
        ++elements;
        EXPECT_TRUE(expected_keys.find(it->key()) != expected_keys.end());
    }
    EXPECT_TRUE(elements == 3);

    // using increment(error_code&)
    elements = 0;
    for (auto it = key_iterator(k); it != key_iterator(); it.increment(ec))
    {
        ++elements;
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(expected_keys.find(it->key()) != expected_keys.end());
    }
    EXPECT_TRUE(elements == 3);
}