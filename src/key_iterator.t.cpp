#include <functional>
#include <set>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/exception.h>
#include <registry/key.h>
#include <registry/key_iterator.h>

using namespace registry;


namespace {

void test_iteration(const key_path& p,
                    const std::function<key_iterator()>& get_iterator)
{
    std::error_code ec;

    std::set<key_path> expected_keys;
    expected_keys.emplace(key_path(p).append("key_1_deep_0"));
    expected_keys.emplace(key_path(p).append("key_2_deep_0"));
    expected_keys.emplace(key_path(p).append("key_3_deep_0"));

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
    expected_keys[0].emplace(key_path(p).append("key_1_deep_0"));
    expected_keys[1].emplace(key_path(p).append("key_1_deep_0\\key_1_deep_1"));
    expected_keys[1].emplace(key_path(p).append("key_1_deep_0\\key_2_deep_1"));
    expected_keys[2].emplace(key_path(p).append("key_1_deep_0\\key_2_deep_1\\key_1_deep_2"));
    expected_keys[2].emplace(key_path(p).append("key_1_deep_0\\key_2_deep_1\\key_2_deep_2"));
    expected_keys[0].emplace(key_path(p).append("key_2_deep_0"));
    expected_keys[0].emplace(key_path(p).append("key_3_deep_0"));
    expected_keys[1].emplace(key_path(p).append("key_3_deep_0\\key_1_deep_1"));
    expected_keys[1].emplace(key_path(p).append("key_3_deep_0\\key_2_deep_1"));

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

    // key_iterator::key_iterator(const key&)
    // key_iterator::key_iterator(const key&, std::error_code&)
    {
        std::error_code ec;
        const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";

        // right permissions
        const key k1(open_only_tag{}, p, access_rights::enumerate_sub_keys | access_rights::query_value);
        key_iterator it1a(k1);
        EXPECT_TRUE(it1a != key_iterator());
        //
        key_iterator it1b(k1, ec);
        EXPECT_TRUE(!ec && it1b != key_iterator());

        // wrong permissions
        const key k2(open_only_tag{}, p, access_rights::set_value);
        EXPECT_THROW(key_iterator it2a(k2), registry_error);
        //
        key_iterator it2b(k2, ec);
        EXPECT_TRUE(ec && it2b == key_iterator());
    }

    // key_iterator::key_iterator(const key_path&)
    // key_iterator::key_iterator(const key_path&, std::error_code&)
    {
        std::error_code ec;

        key_iterator it1("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent");
        EXPECT_TRUE(it1 == key_iterator());

        key_iterator it2("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent", ec);
        EXPECT_TRUE(it2 == key_iterator() && !ec);

        key_iterator it3("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");
        EXPECT_TRUE(it3 != key_iterator());

        key_iterator it4("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read", ec);
        EXPECT_TRUE(it4 != key_iterator() && !ec);
    }
}

TEST(KeyIterator, ConstructFromKeyAndIterate)
{
    const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";
    const key k(open_only_tag{}, p, access_rights::enumerate_sub_keys | access_rights::query_value);

    test_iteration(key_path(), [&]() { return key_iterator(k); });
}

TEST(KeyIterator, ConstructFromPathAndIterate)
{
    std::error_code ec;
    const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";

    test_iteration(p, [&]() { return key_iterator(p); });
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
        const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";

        // right permissions
        const key k1(open_only_tag{}, p, access_rights::enumerate_sub_keys | access_rights::query_value);
        recursive_key_iterator it1a(k1);
        EXPECT_TRUE(it1a != recursive_key_iterator());
        //
        recursive_key_iterator it1b(k1, ec);
        EXPECT_TRUE(!ec && it1b != recursive_key_iterator());

        // wrong permissions
        const key k2(open_only_tag{}, p, access_rights::set_value);
        EXPECT_THROW(recursive_key_iterator it2a(k2), registry_error);
        //
        recursive_key_iterator it2b(k2, ec);
        EXPECT_TRUE(ec && it2b == recursive_key_iterator());

        // wrong permissions (but using key_options::skip_permission_denied)
        const key k3(open_only_tag{}, p, access_rights::set_value);
        recursive_key_iterator it3a(k3, key_options::skip_permission_denied);
        EXPECT_TRUE(it3a == recursive_key_iterator());
        //
        recursive_key_iterator it3b(k3, key_options::skip_permission_denied, ec);
        EXPECT_TRUE(!ec && it3b == recursive_key_iterator());
    }

    // recursive_key_iterator::recursive_key_iterator(const key_path&)
    // recursive_key_iterator::recursive_key_iterator(const key_path&, std::error_code&)
    {
        std::error_code ec;

        recursive_key_iterator it1("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent");
        EXPECT_TRUE(it1 == recursive_key_iterator());

        recursive_key_iterator it2("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent", ec);
        EXPECT_TRUE(it2 == recursive_key_iterator() && !ec);

        recursive_key_iterator it3("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");
        EXPECT_TRUE(it3 != recursive_key_iterator());
        EXPECT_TRUE(it3.options() == key_options::none);

        recursive_key_iterator it4("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read", ec);
        EXPECT_TRUE(it4 != recursive_key_iterator() && !ec);
        EXPECT_TRUE(it4.options() == key_options::none);

        // TODO: test construction with key_options::skip_permission_denied
    }
}

TEST(RecursiveKeyIterator, ConstructFromKeyAndIterate)
{
    const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";

    test_recursive_iteration(p, [&]() { return recursive_key_iterator(p); });
}

TEST(RecursiveKeyIterator, ConstructFromPathAndIterate)
{
    const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";
    const key k(open_only_tag{}, p, access_rights::enumerate_sub_keys | access_rights::query_value);

    test_recursive_iteration(p, [&]() { return recursive_key_iterator(k); });
}

TEST(RecursiveKeyIterator, Pop)
{
    const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read";

    std::set<key_path> expected_keys;
    expected_keys.emplace(key_path(p).append("key_1_deep_0"));
    expected_keys.emplace(key_path(p).append("key_2_deep_0"));
    expected_keys.emplace(key_path(p).append("key_3_deep_0"));

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