#include <functional>
#include <set>
#include <vector>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key.h>
#include <registry/key_iterator.h>

using namespace registry;


namespace {

void test_iteration(const key_path& p,
                    const std::function<key_iterator()>& get_iterator)
{
    std::error_code ec;

    std::set<key_path> expected_keys;
    expected_keys.emplace(key_path(p).append(TEXT("key_1_deep_0")));
    expected_keys.emplace(key_path(p).append(TEXT("key_2_deep_0")));
    expected_keys.emplace(key_path(p).append(TEXT("key_3_deep_0")));

    // using range-based for loop
    int elements = 0;
    for (const auto& entry : get_iterator())
    {
        ++elements;
        EXPECT_TRUE(expected_keys.find(entry.path()) != expected_keys.end());
    }
    EXPECT_TRUE(elements == 3);

    // using operator++()
    elements = 0;
    for (auto it = get_iterator(); it != key_iterator(); ++it)
    {
        ++elements;
        EXPECT_TRUE(expected_keys.find(it->path()) != expected_keys.end());
    }
    EXPECT_TRUE(elements == 3);

    // using operator++(int)
    elements = 0;
    for (auto it = get_iterator(); it != key_iterator(); it++)
    {
        ++elements;
        EXPECT_TRUE(expected_keys.find(it->path()) != expected_keys.end());
    }
    EXPECT_TRUE(elements == 3);

    // using increment(error_code&)
    elements = 0;
    for (auto it = get_iterator(); it != key_iterator(); it.increment(ec))
    {
        ++elements;
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(expected_keys.find(it->path()) != expected_keys.end());
    }
    EXPECT_TRUE(elements == 3);
}

void test_recursive_iteration(const key_path& p,
                              const std::function<recursive_key_iterator()>& get_iterator)
{
    std::error_code ec;

    // NOTE: vector element index represent the depth
    std::vector<std::set<key_path>> expected_keys(3);
    expected_keys[0].emplace(key_path(p).append(TEXT("key_1_deep_0")));
    expected_keys[1].emplace(key_path(p).append(TEXT("key_1_deep_0\\key_1_deep_1")));
    expected_keys[1].emplace(key_path(p).append(TEXT("key_1_deep_0\\key_2_deep_1")));
    expected_keys[2].emplace(key_path(p).append(TEXT("key_1_deep_0\\key_2_deep_1\\key_1_deep_2")));
    expected_keys[2].emplace(key_path(p).append(TEXT("key_1_deep_0\\key_2_deep_1\\key_2_deep_2")));
    expected_keys[0].emplace(key_path(p).append(TEXT("key_2_deep_0")));
    expected_keys[0].emplace(key_path(p).append(TEXT("key_3_deep_0")));
    expected_keys[1].emplace(key_path(p).append(TEXT("key_3_deep_0\\key_1_deep_1")));
    expected_keys[1].emplace(key_path(p).append(TEXT("key_3_deep_0\\key_2_deep_1")));

    // using range-based for loop
    int elements = 0;
    for (const auto& entry : recursive_key_iterator(p))
    {
        ++elements;
        EXPECT_TRUE(expected_keys.at(0).find(entry.path()) != expected_keys.at(0).end() ||
                    expected_keys.at(1).find(entry.path()) != expected_keys.at(1).end() ||
                    expected_keys.at(2).find(entry.path()) != expected_keys.at(2).end());
    }
    EXPECT_TRUE(elements == 9);

    // using operator++()
    elements = 0;
    for (auto it = recursive_key_iterator(p); it != recursive_key_iterator(); ++it)
    {
        ++elements;
        EXPECT_TRUE(expected_keys.at(it.depth()).find(it->path()) != expected_keys.at(it.depth()).end());
    }
    EXPECT_TRUE(elements == 9);

    // using operator++(int)
    elements = 0;
    for (auto it = recursive_key_iterator(p); it != recursive_key_iterator(); it++)
    {
        ++elements;
        EXPECT_TRUE(expected_keys.at(it.depth()).find(it->path()) != expected_keys.at(it.depth()).end());
    }
    EXPECT_TRUE(elements == 9);

    // using increment(error_code&)
    elements = 0;
    for (auto it = recursive_key_iterator(p); it != recursive_key_iterator(); it.increment(ec))
    {
        ++elements;
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(expected_keys.at(it.depth()).find(it->path()) != expected_keys.at(it.depth()).end());
    }
    EXPECT_TRUE(elements == 9);
}

}


TEST(KeyIterator, Construct)
{
    // default constructor
    {
        key_iterator it;
        EXPECT_TRUE(it == key_iterator());
    }

    // key_iterator::key_iterator(const key_path&)
    // key_iterator::key_iterator(const key_path&, std::error_code&)
    {
        std::error_code ec;

        key_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"));
        EXPECT_TRUE(it1 == key_iterator());

        key_iterator it2(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"), ec);
        EXPECT_TRUE(it2 == key_iterator() && !ec);

        key_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"));
        EXPECT_TRUE(it3 != key_iterator());

        key_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), ec);
        EXPECT_TRUE(it4 != key_iterator() && !ec);
    }

    // key_iterator::key_iterator(const key&)
    // key_iterator::key_iterator(const key&, std::error_code&)
    {
        // TODO: ...

        //std::error_code ec;
        //const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

        //key_iterator it1(key_handle(p, access_rights::enumerate_sub_keys | access_rights::query_value));
        //key_iterator it2(key_handle(p, access_rights::enumerate_sub_keys | access_rights::query_value), ec);
        //EXPECT_TRUE(it1 != key_iterator());
        //EXPECT_TRUE(it2 != key_iterator() && !ec);
    }
}

TEST(KeyIterator, ObtainFromPathAndIterate)
{
    std::error_code ec;
    const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

    test_iteration(p, [&]() { return key_iterator(p); });
}

TEST(KeyIterator, ObtainFromKeyAndIterate)
{
    const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");
    const key k(open_only_tag{}, p, access_rights::enumerate_sub_keys | access_rights::query_value);

    test_iteration(key_path(), [&]() { return key_iterator(k); });
}

TEST(RecursiveKeyIterator, Construct)
{
    // default constructor
    {
        recursive_key_iterator it;
        EXPECT_TRUE(it == recursive_key_iterator());
    }

    // recursive_key_iterator::recursive_key_iterator(const key_path&)
    // recursive_key_iterator::recursive_key_iterator(const key_path&, std::error_code&)
    {
        std::error_code ec;

        recursive_key_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"));
        EXPECT_TRUE(it1 == recursive_key_iterator());

        recursive_key_iterator it2(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"), ec);
        EXPECT_TRUE(it2 == recursive_key_iterator() && !ec);

        recursive_key_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"));
        EXPECT_TRUE(it3 != recursive_key_iterator());
        EXPECT_TRUE(it3.options() == key_options::none);

        recursive_key_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), ec);
        EXPECT_TRUE(it4 != recursive_key_iterator() && !ec);
        EXPECT_TRUE(it4.options() == key_options::none);

        // TODO: test construction with key_options::skip_permission_denied
    }

    // recursive_key_iterator::recursive_key_iterator(const key_handle&)
    // recursive_key_iterator::recursive_key_iterator(const key_handle&, std::error_code&)
    {
        // TODO: ...
    }
}

TEST(RecursiveKeyIterator, ConstructFromPathAndIterate)
{
    const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

    test_recursive_iteration(p, [&]() { return recursive_key_iterator(p); });
}

TEST(RecursiveKeyIterator, ConstructFromKeyAndIterate)
{
    // TODO: ...
}

TEST(RecursiveKeyIterator, Pop)
{
    const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

    std::set<key_path> expected_keys;
    expected_keys.emplace(key_path(p).append(TEXT("key_1_deep_0")));
    expected_keys.emplace(key_path(p).append(TEXT("key_2_deep_0")));
    expected_keys.emplace(key_path(p).append(TEXT("key_3_deep_0")));

    int elements = 0;
    for (auto it = recursive_key_iterator(p); ; ++it)
    {
        if (it.depth()) it.pop();
        if (it == recursive_key_iterator()) break;

        ++elements;
        EXPECT_TRUE(expected_keys.find(it->path()) != expected_keys.end());
        
    }
    EXPECT_TRUE(elements == 3);

    elements = 0;
    std::error_code ec;
    for (auto it = recursive_key_iterator(p); ; ++it)
    {
        if (it.depth()) it.pop(ec);
        if (it == recursive_key_iterator()) break;

        ++elements;
        EXPECT_TRUE(!ec && expected_keys.find(it->path()) != expected_keys.end());
        
    }
    EXPECT_TRUE(elements == 3);
}