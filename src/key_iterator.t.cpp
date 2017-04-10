#include <set>
#include <vector>
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
        EXPECT_TRUE(it == key_iterator());
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
    expected_keys.emplace(key(k).append(TEXT("key_1_deep_0")));
    expected_keys.emplace(key(k).append(TEXT("key_2_deep_0")));
    expected_keys.emplace(key(k).append(TEXT("key_3_deep_0")));

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

TEST(RecursiveKeyIterator, Construct)
{
    // default constructor
    {
        recursive_key_iterator it;
        EXPECT_TRUE(it == recursive_key_iterator());
    }

    // recursive_key_iterator::recursive_key_iterator(const key&)
    // recursive_key_iterator::recursive_key_iterator(const key&, std::error_code&)
    {
        std::error_code ec;

        recursive_key_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"));
        recursive_key_iterator it2(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"), ec);
        EXPECT_TRUE(it1 == recursive_key_iterator());
        EXPECT_TRUE(it2 == recursive_key_iterator() && !ec);

        recursive_key_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"));
        recursive_key_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), ec);
        EXPECT_TRUE(it3 != recursive_key_iterator());
        EXPECT_TRUE(it4 != recursive_key_iterator() && !ec);
    }

    // recursive_key_iterator::recursive_key_iterator(const key_handle&)
    // recursive_key_iterator::recursive_key_iterator(const key_handle&, std::error_code&)
    {
        // TODO: ...
    }
}

TEST(RecursiveKeyIterator, Iterate)
{
    std::error_code ec;
    const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

    // NOTE: vector element index represent the depth
    std::vector<std::set<key>> expected_keys(3);
    expected_keys[0].emplace(key(k).append(TEXT("key_1_deep_0")));
    expected_keys[1].emplace(key(k).append(TEXT("key_1_deep_0\\key_1_deep_1")));
    expected_keys[1].emplace(key(k).append(TEXT("key_1_deep_0\\key_2_deep_1")));
    expected_keys[2].emplace(key(k).append(TEXT("key_1_deep_0\\key_2_deep_1\\key_1_deep_2")));
    expected_keys[2].emplace(key(k).append(TEXT("key_1_deep_0\\key_2_deep_1\\key_2_deep_2")));
    expected_keys[0].emplace(key(k).append(TEXT("key_2_deep_0")));
    expected_keys[0].emplace(key(k).append(TEXT("key_3_deep_0")));
    expected_keys[1].emplace(key(k).append(TEXT("key_3_deep_0\\key_1_deep_1")));
    expected_keys[1].emplace(key(k).append(TEXT("key_3_deep_0\\key_2_deep_1")));

    // using range-based for loop
    int elements = 0;
    for (const auto& entry : recursive_key_iterator(k))
    {
        ++elements;
        EXPECT_TRUE(expected_keys.at(0).find(entry.key()) != expected_keys.at(0).end() ||
                    expected_keys.at(1).find(entry.key()) != expected_keys.at(1).end() ||
                    expected_keys.at(2).find(entry.key()) != expected_keys.at(2).end());
    }
    EXPECT_TRUE(elements == 9);

    // using operator++()
    elements = 0;
    for (auto it = recursive_key_iterator(k); it != recursive_key_iterator(); ++it)
    {
        ++elements;
        EXPECT_TRUE(expected_keys.at(it.depth()).find(it->key()) != expected_keys.at(it.depth()).end());
    }
    EXPECT_TRUE(elements == 9);

    // using operator++(int)
    elements = 0;
    for (auto it = recursive_key_iterator(k); it != recursive_key_iterator(); it++)
    {
        ++elements;
        EXPECT_TRUE(expected_keys.at(it.depth()).find(it->key()) != expected_keys.at(it.depth()).end());
    }
    EXPECT_TRUE(elements == 9);

    // using increment(error_code&)
    elements = 0;
    for (auto it = recursive_key_iterator(k); it != recursive_key_iterator(); it.increment(ec))
    {
        ++elements;
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(expected_keys.at(it.depth()).find(it->key()) != expected_keys.at(it.depth()).end());
    }
    EXPECT_TRUE(elements == 9);
}