#include <iterator>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key.h>

using namespace registry;


TEST(Key, Construct) 
{
    key k1;
    ASSERT_TRUE(k1 == key(string_type()));

    key k2(TEXT("HKEY_CURRENT_user\\Test"), view::view_32bit);
    EXPECT_TRUE(k2.name() == TEXT("HKEY_CURRENT_user\\Test"));
    EXPECT_TRUE(k2.view() == view::view_32bit);
}

TEST(Key, Assign)
{
    key k1(TEXT("Test1"), view::view_32bit);
    key k2(TEXT("Test1\\Test2\\Test3"), view::view_64bit);

    ASSERT_TRUE(k1.assign(k2.name(), k2.view()) == k2);
}

TEST(Key, FromKeyId)
{
    static const auto test = [](key_id id, string_view_type expected_name)
    {
        const key k = key::from_key_id(id);
        ASSERT_TRUE(k.name() == expected_name);
        ASSERT_TRUE(k.view() == key::default_view);
        ASSERT_TRUE(k.root_key_id() == id);
    };

    test(key_id::classes_root,                TEXT("HKEY_CLASSES_ROOT"));
    test(key_id::current_user,                TEXT("HKEY_CURRENT_USER"));
    test(key_id::local_machine,               TEXT("HKEY_LOCAL_MACHINE"));
    test(key_id::users,                       TEXT("HKEY_USERS"));
    test(key_id::performance_data,            TEXT("HKEY_PERFORMANCE_DATA"));
    test(key_id::performance_text,            TEXT("HKEY_PERFORMANCE_TEXT"));
    test(key_id::performance_nlstext,         TEXT("HKEY_PERFORMANCE_NLSTEXT"));
    test(key_id::current_config,              TEXT("HKEY_CURRENT_CONFIG"));
    test(key_id::current_user_local_settings, TEXT("HKEY_CURRENT_USER_LOCAL_SETTINGS"));
    test(key_id::unknown,                     TEXT(""));
}

TEST(Key, Compare)
{
    ASSERT_TRUE(key().compare(key()) == 0);

    ASSERT_TRUE(key(TEXT("AAA")).compare(key(TEXT("AAA"))) == 0);

    ASSERT_TRUE(key(TEXT("AAA\\BBB")).compare(key(TEXT("AAA\\BBB"))) == 0);

    ASSERT_TRUE(key(TEXT("AAA\\BBB")).compare(key(TEXT("aAa\\\\bBb\\"))) == 0);

    ASSERT_TRUE(key(TEXT("AAA\\BBB")).compare(key(TEXT("AAA\\CCC"))) < 0);

    ASSERT_TRUE(key(TEXT("AAA\\BBB")).compare(key(TEXT("AAA\\AAA"))) > 0);

    ASSERT_TRUE(key(TEXT("AAA\\AAA"), view::view_32bit).compare(key(TEXT("AAA\\BBB"), view::view_64bit)) > 0);

    ASSERT_TRUE(key(TEXT("AAA\\BBB"), view::view_64bit).compare(key(TEXT("AAA\\AAA"), view::view_32bit)) < 0);
}

TEST(Key, Iterate)
{
    key k01;
    ASSERT_TRUE(k01.begin() == k01.end());

    key k02 = TEXT("\\");
    ASSERT_TRUE(k02.begin() == k02.end());

    key k03 = TEXT("\\\\");
    ASSERT_TRUE(k03.begin() == k03.end());

    key k04 = TEXT("Test");
    ASSERT_TRUE(std::distance(k04.begin(), k04.end()) == 1);
    ASSERT_TRUE(*k04.begin() == TEXT("Test"));

    key k05 = TEXT("\\Test");
    ASSERT_TRUE(std::distance(k05.begin(), k05.end()) == 1);
    ASSERT_TRUE(*k05.begin() == TEXT("Test"));

    key k06 = TEXT("Test\\");
    ASSERT_TRUE(std::distance(k06.begin(), k06.end()) == 1);
    ASSERT_TRUE(*k06.begin() == TEXT("Test"));

    key k07 = TEXT("\\\\Test\\\\");
    ASSERT_TRUE(std::distance(k07.begin(), k07.end()) == 1);
    ASSERT_TRUE(*k07.begin() == TEXT("Test"));

    key k08 = TEXT("Test1\\Test2\\Test3");
    ASSERT_TRUE(std::distance(k08.begin(), k08.end()) == 3);
    ASSERT_TRUE(*k08.begin() == TEXT("Test1"));
    ASSERT_TRUE(*(++k08.begin()) == TEXT("Test2"));
    ASSERT_TRUE(*(++(++k08.begin())) == TEXT("Test3"));

    key k09 = TEXT("\\Test1\\Test2\\Test3");
    ASSERT_TRUE(std::distance(k09.begin(), k09.end()) == 3);
    ASSERT_TRUE(*k09.begin() == TEXT("Test1"));
    ASSERT_TRUE(*(++k09.begin()) == TEXT("Test2"));
    ASSERT_TRUE(*(++(++k09.begin())) == TEXT("Test3"));

    key k10 = TEXT("Test1\\Test2\\Test3\\");
    ASSERT_TRUE(std::distance(k10.begin(), k10.end()) == 3);
    ASSERT_TRUE(*k10.begin() == TEXT("Test1"));
    ASSERT_TRUE(*(++k10.begin()) == TEXT("Test2"));
    ASSERT_TRUE(*(++(++k10.begin())) == TEXT("Test3"));

    key k11 = TEXT("\\\\Test1\\Test2\\\\Test3\\\\");
    ASSERT_TRUE(std::distance(k11.begin(), k11.end()) == 3);
    ASSERT_TRUE(*k11.begin() == TEXT("Test1"));
    ASSERT_TRUE(*(++k11.begin()) == TEXT("Test2"));
    ASSERT_TRUE(*(++(++k11.begin())) == TEXT("Test3"));

    key k12 = TEXT("Test1\\Test2\\Test3");
    auto k12_it1 = k12.begin();
    std::advance(k12_it1, 3);
    ASSERT_TRUE(k12_it1 == k12.end());

    auto k12_it2 = k12.end();
    ASSERT_TRUE(*--k12_it2 == TEXT("Test3"));
    ASSERT_TRUE(*--k12_it2 == TEXT("Test2"));
    ASSERT_TRUE(*--k12_it2 == TEXT("Test1"));
}

TEST(Key, Queries)
{
    // key::has_root_key()
    {
        key k1;
        ASSERT_TRUE(k1.has_root_key() == (k1.begin() != k1.end()));

        key k2 = TEXT("Test");
        ASSERT_TRUE(k2.has_root_key() == (k2.begin() != k2.end()));

        key k3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(k3.has_root_key() == (k3.begin() != k3.end()));
    }

    // key::root_key_id()
    {
        ASSERT_TRUE(key().root_key_id() == key_id::unknown);

        ASSERT_TRUE(key(TEXT("HKEY_CLASSES_root")).root_key_id() == key_id::classes_root);

        ASSERT_TRUE(key(TEXT("HKEY_CURRENT_user")).root_key_id() == key_id::current_user);

        ASSERT_TRUE(key(TEXT("HKEY_LOCAL_machine")).root_key_id() == key_id::local_machine);

        ASSERT_TRUE(key(TEXT("HKEY_users")).root_key_id() == key_id::users);

        ASSERT_TRUE(key(TEXT("HKEY_PERFORMANCE_data")).root_key_id() == key_id::performance_data);

        ASSERT_TRUE(key(TEXT("HKEY_PERFORMANCE_text")).root_key_id() == key_id::performance_text);

        ASSERT_TRUE(key(TEXT("HKEY_PERFORMANCE_nlstext")).root_key_id() == key_id::performance_nlstext);

        ASSERT_TRUE(key(TEXT("HKEY_CURRENT_config")).root_key_id() == key_id::current_config);

        ASSERT_TRUE(key(TEXT("HKEY_CURRENT_USER_LOCAL_settings")).root_key_id() == key_id::current_user_local_settings);
    }

    // key::has_leaf_key()
    {
        key k1;
        ASSERT_TRUE(k1.has_leaf_key() == (k1.begin() != k1.end()));

        key k2 = TEXT("Test");
        ASSERT_TRUE(k2.has_leaf_key() == (k2.begin() != k2.end()));

        key k3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(k3.has_leaf_key() == (k3.begin() != k3.end()));
    }

    // key::has_parent_key()
    {
        key k1;
        ASSERT_TRUE(k1.has_parent_key() == (k1.has_root_key() && ++k1.begin() != k1.end()));

        key k2 = TEXT("Test");
        ASSERT_TRUE(k2.has_parent_key() == (k2.has_root_key() && ++k2.begin() != k2.end()));

        key k3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(k3.has_parent_key() == (k3.has_root_key() && ++k3.begin() != k3.end()));
    }

    // key::is_absolute()
    // key::is_relative()
    {
        key k1;
        ASSERT_TRUE(!k1.is_absolute());
        ASSERT_TRUE(k1.is_relative() == !k1.is_absolute());

        key k2 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(!k2.is_absolute());
        ASSERT_TRUE(k2.is_relative() == !k2.is_absolute());

        key k3 = TEXT("\\HKEY_CURRENT_USER\\Test2\\Test3");
        ASSERT_TRUE(!k3.is_absolute());
        ASSERT_TRUE(k3.is_relative() == !k3.is_absolute());

        key k4 = TEXT("HKEY_CURRENT_USER\\Test2\\Test3");
        ASSERT_TRUE(k4.is_absolute());
        ASSERT_TRUE(k4.is_relative() == !k4.is_absolute());
    }
}

TEST(Key, Decomposition)
{
    // ket::root_key()
    {
        key k1;
        ASSERT_TRUE(k1.root_key() == (k1.has_root_key() ? key(*k1.begin(), k1.view()) : key(string_type(), k1.view())));

        key k2 = TEXT("Test");
        ASSERT_TRUE(k2.root_key() == (k2.has_root_key() ? key(*k2.begin(), k2.view()) : key(string_type(), k2.view())));

        key k3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(k3.root_key() == (k3.has_root_key() ? key(*k3.begin(), k3.view()) : key(string_type(), k3.view())));
    }

    // ket::leaf_key()
    {
        key k1;
        ASSERT_TRUE(k1.leaf_key() == (k1.has_leaf_key() ? key(*--k1.end(), k1.view()) : key(string_type(), k1.view())));

        key k2 = TEXT("Test");
        ASSERT_TRUE(k2.leaf_key() == (k2.has_leaf_key() ? key(*--k2.end(), k2.view()) : key(string_type(), k2.view())));

        key k3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(k3.leaf_key() == (k3.has_leaf_key() ? key(*--k3.end(), k3.view()) : key(string_type(), k3.view())));
    }

    // ket::parent_key()
    {
        key k1;
        ASSERT_TRUE(k1.parent_key() == key(string_type(), k1.view()));

        key k2 = TEXT("Test");
        ASSERT_TRUE(k2.parent_key() == key(string_type(), k2.view()));

        key k3 = TEXT("Test1\\Test2\\\\Test3\\");
        ASSERT_TRUE(k3.parent_key() == key(TEXT("Test1\\Test2"), k3.view()));
    }
}

TEST(Key, Modifiers)
{
    // key::append(const Source&)
    {
        // redundant separator case 1
        key k1 = TEXT("HKEY_CURRENT_USER\\");
        ASSERT_TRUE(k1.append(TEXT("Test")).name() == TEXT("HKEY_CURRENT_USER\\Test"));

        // redundant separator case 2
        key k2 = TEXT("");
        key k3 = TEXT("\\\\");
        ASSERT_TRUE(k2.append(TEXT("Test")).name() == TEXT("Test"));
        ASSERT_TRUE(k3.append(TEXT("Test")).name() == TEXT("\\\\Test"));

        // redundant separator case 3
        key k4 = TEXT("HKEY_CURRENT_USER");
        ASSERT_TRUE(k4.append(TEXT("")).name() == TEXT("HKEY_CURRENT_USER"));

        // redundant separator case 4
        key k5 = TEXT("HKEY_CURRENT_USER");
        ASSERT_TRUE(k5.append(TEXT("\\Test")).name() == TEXT("HKEY_CURRENT_USER\\Test"));

        // adds a separator
        key k6 = TEXT("HKEY_CURRENT_USER");
        ASSERT_TRUE(k6.append(TEXT("Test")).name() == TEXT("HKEY_CURRENT_USER\\Test"));
    }

    // key::append(const key&)
    {
        key k1(TEXT("HKEY_CURRENT_USER"), view::view_32bit);
        key k2(TEXT("Test1\\Test2\\\\"), view::view_64bit);

        k1.append(k2);
        ASSERT_TRUE(k1.name() == TEXT("HKEY_CURRENT_USER\\Test1\\Test2") && k1.view() == view::view_64bit);
    }

    // key::concat(string_view_type)
    {
        key k1, k1_copy = k1;
        ASSERT_TRUE(k1.concat(TEXT("Test")) == key(k1_copy.name() + TEXT("Test"), k1_copy.view()));

        key k2 = TEXT("HKEY_CURRENT_USER"), k2_copy = k2;
        ASSERT_TRUE(k2.concat(TEXT("Test")) == key(k2_copy.name() + TEXT("Test"), k2_copy.view()));

        key k3 = TEXT("HKEY_CURRENT_USER\\"), k3_copy = k3;
        ASSERT_TRUE(k3.concat(TEXT("Test")) == key(k3_copy.name() + TEXT("Test"), k3_copy.view()));
    }

    // key::remove_leaf()
    {
        key k1 = TEXT("HKEY_CURRENT_USER");
        ASSERT_TRUE(k1.remove_leaf().name() == TEXT(""));

        key k2 = TEXT("HKEY_CURRENT_USER\\");
        ASSERT_TRUE(k2.remove_leaf().name() == TEXT(""));

        key k3 = TEXT("\\HKEY_CURRENT_USER\\");
        ASSERT_TRUE(k3.remove_leaf().name() == TEXT(""));

        key k4 = TEXT("HKEY_CURRENT_USER\\Test");
        ASSERT_TRUE(k4.remove_leaf().name() == TEXT("HKEY_CURRENT_USER"));

        key k5 = TEXT("HKEY_CURRENT_USER\\Test\\\\");
        ASSERT_TRUE(k5.remove_leaf().name() == TEXT("HKEY_CURRENT_USER"));
    }

    // key::replace_leaf(string_view_type)
    {
        key k1 = TEXT("HKEY_CURRENT_USER"), k1_copy = k1;
        ASSERT_TRUE(k1.replace_leaf(TEXT("replacement")) == k1_copy.remove_leaf().append(TEXT("replacement")));

        key k2 = TEXT("HKEY_CURRENT_USER\\"), k2_copy = k2;
        ASSERT_TRUE(k2.replace_leaf(TEXT("replacement")) == k2_copy.remove_leaf().append(TEXT("replacement")));

        key k3 = TEXT("\\HKEY_CURRENT_USER\\"), k3_copy = k3;
        ASSERT_TRUE(k3.replace_leaf(TEXT("replacement")) == k3_copy.remove_leaf().append(TEXT("replacement")));

        key k4 = TEXT("HKEY_CURRENT_USER\\Test"), k4_copy = k4;
        ASSERT_TRUE(k4.replace_leaf(TEXT("replacement")) == k4_copy.remove_leaf().append(TEXT("replacement")));

        key k5 = TEXT("HKEY_CURRENT_USER\\Test\\\\"), k5_copy = k5;
        ASSERT_TRUE(k5.replace_leaf(TEXT("replacement")) == k5_copy.remove_leaf().append(TEXT("replacement")));
    }
}

TEST(Key, Swap)
{
    key k1(TEXT("HKEY_CURRENT_USER\\Test"), view::view_32bit), k1_copy = k1;
    key k2(TEXT("HKEY_LOCAL_MACHINE\\Test"), view::view_64bit), k2_copy = k2;

    swap(k1, k2);
    ASSERT_TRUE(k1 == k2_copy && k2 == k1_copy);
}

TEST(Key, Hash)
{
    key k01, k02;
    ASSERT_TRUE(hash_value(k01) == hash_value(k02));

    key k03 = TEXT("HKEY_CURRENT_USER\\Test"), k04 = k03;
    ASSERT_TRUE(hash_value(k03) == hash_value(k04));

    key k05 = TEXT("HKEY_CURRENT_USER\\Test");
    key k06 = TEXT("HKEY_CURRENT_user\\\\Test\\");
    ASSERT_TRUE(hash_value(k05) == hash_value(k06));
}