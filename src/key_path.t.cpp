#include <iterator>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key_path.h>

using namespace registry;


TEST(KeyPath, Construct) 
{
    // default constructor
    key_path p1;
    ASSERT_TRUE(p1.key_name().empty());
    ASSERT_TRUE(p1.key_view() == view::view_default);

    // construct from name an view
    key_path p2(TEXT("HKEY_CURRENT_user\\Test"), view::view_32bit);
    EXPECT_TRUE(p2.key_name() == TEXT("HKEY_CURRENT_user\\Test"));
    EXPECT_TRUE(p2.key_view() == view::view_32bit);

    // test implicit from-string construction
    key_path p3 = TEXT("HKEY_CURRENT_user\\Test");
    EXPECT_TRUE(p2.key_name() == TEXT("HKEY_CURRENT_user\\Test"));
    EXPECT_TRUE(p2.key_view() == view::view_default);
}

TEST(KeyPath, Assign)
{
    key_path p1(TEXT("Test1"), view::view_32bit);
    key_path p2(TEXT("Test1\\Test2\\Test3"), view::view_64bit);

    ASSERT_TRUE(p1.assign(p2.key_name(), p2.key_view()) == p2);
}

TEST(KeyPath, FromKeyId)
{
    static const auto test = [](key_id id, string_view_type expected_name)
    {
        const key_path p = key_path::from_key_id(id);
        ASSERT_TRUE(p.key_name() == expected_name);
        ASSERT_TRUE(p.key_view() == view::view_default);
        ASSERT_TRUE(p.root_key_id() == id);
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

TEST(KeyPath, Compare)
{
    ASSERT_TRUE(key_path().compare(key_path()) == 0);

    ASSERT_TRUE(key_path(TEXT("AAA")).compare(key_path(TEXT("AAA"))) == 0);

    ASSERT_TRUE(key_path(TEXT("AAA\\BBB")).compare(key_path(TEXT("AAA\\BBB"))) == 0);

    ASSERT_TRUE(key_path(TEXT("AAA\\BBB")).compare(key_path(TEXT("aAa\\\\bBb\\"))) == 0);

    ASSERT_TRUE(key_path(TEXT("AAA\\BBB")).compare(key_path(TEXT("AAA\\CCC"))) < 0);

    ASSERT_TRUE(key_path(TEXT("AAA\\BBB")).compare(key_path(TEXT("AAA\\AAA"))) > 0);

    ASSERT_TRUE(key_path(TEXT("AAA\\AAA"), view::view_32bit).compare(key_path(TEXT("AAA\\BBB"), view::view_64bit)) > 0);

    ASSERT_TRUE(key_path(TEXT("AAA\\BBB"), view::view_64bit).compare(key_path(TEXT("AAA\\AAA"), view::view_32bit)) < 0);
}

TEST(KeyPath, Iterate)
{
    key_path p01;
    ASSERT_TRUE(p01.begin() == p01.end());

    key_path p02 = TEXT("\\");
    ASSERT_TRUE(p02.begin() == p02.end());

    key_path p03 = TEXT("\\\\");
    ASSERT_TRUE(p03.begin() == p03.end());

    key_path p04 = TEXT("Test");
    ASSERT_TRUE(std::distance(p04.begin(), p04.end()) == 1);
    ASSERT_TRUE(*p04.begin() == TEXT("Test"));

    key_path p05 = TEXT("\\Test");
    ASSERT_TRUE(std::distance(p05.begin(), p05.end()) == 1);
    ASSERT_TRUE(*p05.begin() == TEXT("Test"));

    key_path p06 = TEXT("Test\\");
    ASSERT_TRUE(std::distance(p06.begin(), p06.end()) == 1);
    ASSERT_TRUE(*p06.begin() == TEXT("Test"));

    key_path p07 = TEXT("\\\\Test\\\\");
    ASSERT_TRUE(std::distance(p07.begin(), p07.end()) == 1);
    ASSERT_TRUE(*p07.begin() == TEXT("Test"));

    key_path p08 = TEXT("Test1\\Test2\\Test3");
    ASSERT_TRUE(std::distance(p08.begin(), p08.end()) == 3);
    ASSERT_TRUE(*p08.begin() == TEXT("Test1"));
    ASSERT_TRUE(*(++p08.begin()) == TEXT("Test2"));
    ASSERT_TRUE(*(++(++p08.begin())) == TEXT("Test3"));

    key_path p09 = TEXT("\\Test1\\Test2\\Test3");
    ASSERT_TRUE(std::distance(p09.begin(), p09.end()) == 3);
    ASSERT_TRUE(*p09.begin() == TEXT("Test1"));
    ASSERT_TRUE(*(++p09.begin()) == TEXT("Test2"));
    ASSERT_TRUE(*(++(++p09.begin())) == TEXT("Test3"));

    key_path p10 = TEXT("Test1\\Test2\\Test3\\");
    ASSERT_TRUE(std::distance(p10.begin(), p10.end()) == 3);
    ASSERT_TRUE(*p10.begin() == TEXT("Test1"));
    ASSERT_TRUE(*(++p10.begin()) == TEXT("Test2"));
    ASSERT_TRUE(*(++(++p10.begin())) == TEXT("Test3"));

    key_path p11 = TEXT("\\\\Test1\\Test2\\\\Test3\\\\");
    ASSERT_TRUE(std::distance(p11.begin(), p11.end()) == 3);
    ASSERT_TRUE(*p11.begin() == TEXT("Test1"));
    ASSERT_TRUE(*(++p11.begin()) == TEXT("Test2"));
    ASSERT_TRUE(*(++(++p11.begin())) == TEXT("Test3"));

    key_path p12 = TEXT("Test1\\Test2\\Test3");
    auto p12_it1 = p12.begin();
    std::advance(p12_it1, 3);
    ASSERT_TRUE(p12_it1 == p12.end());

    auto p12_it2 = p12.end();
    ASSERT_TRUE(*--p12_it2 == TEXT("Test3"));
    ASSERT_TRUE(*--p12_it2 == TEXT("Test2"));
    ASSERT_TRUE(*--p12_it2 == TEXT("Test1"));
}

TEST(KeyPath, Queries)
{
    // key_path::has_root_key()
    {
        key_path p1;
        ASSERT_TRUE(p1.has_root_key() == (p1.begin() != p1.end()));

        key_path p2 = TEXT("Test");
        ASSERT_TRUE(p2.has_root_key() == (p2.begin() != p2.end()));

        key_path p3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(p3.has_root_key() == (p3.begin() != p3.end()));
    }

    // key_path::root_key_id()
    {
        ASSERT_TRUE(key_path().root_key_id() == key_id::unknown);

        ASSERT_TRUE(key_path(TEXT("HKEY_CLASSES_root")).root_key_id() == key_id::classes_root);

        ASSERT_TRUE(key_path(TEXT("HKEY_CURRENT_user")).root_key_id() == key_id::current_user);

        ASSERT_TRUE(key_path(TEXT("HKEY_LOCAL_machine")).root_key_id() == key_id::local_machine);

        ASSERT_TRUE(key_path(TEXT("HKEY_users")).root_key_id() == key_id::users);

        ASSERT_TRUE(key_path(TEXT("HKEY_PERFORMANCE_data")).root_key_id() == key_id::performance_data);

        ASSERT_TRUE(key_path(TEXT("HKEY_PERFORMANCE_text")).root_key_id() == key_id::performance_text);

        ASSERT_TRUE(key_path(TEXT("HKEY_PERFORMANCE_nlstext")).root_key_id() == key_id::performance_nlstext);

        ASSERT_TRUE(key_path(TEXT("HKEY_CURRENT_config")).root_key_id() == key_id::current_config);

        ASSERT_TRUE(key_path(TEXT("HKEY_CURRENT_USER_LOCAL_settings")).root_key_id() == key_id::current_user_local_settings);
    }

    // key_path::has_leaf_key()
    {
        key_path p1;
        ASSERT_TRUE(p1.has_leaf_key() == (p1.begin() != p1.end()));

        key_path p2 = TEXT("Test");
        ASSERT_TRUE(p2.has_leaf_key() == (p2.begin() != p2.end()));

        key_path p3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(p3.has_leaf_key() == (p3.begin() != p3.end()));
    }

    // key_path::has_parent_key()
    {
        key_path p1;
        ASSERT_TRUE(p1.has_parent_key() == (p1.has_root_key() && ++p1.begin() != p1.end()));

        key_path p2 = TEXT("Test");
        ASSERT_TRUE(p2.has_parent_key() == (p2.has_root_key() && ++p2.begin() != p2.end()));

        key_path p3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(p3.has_parent_key() == (p3.has_root_key() && ++p3.begin() != p3.end()));
    }

    // key_path::is_absolute()
    // key_path::is_relative()
    {
        key_path p1;
        ASSERT_TRUE(!p1.is_absolute());
        ASSERT_TRUE(p1.is_relative() == !p1.is_absolute());

        key_path p2 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(!p2.is_absolute());
        ASSERT_TRUE(p2.is_relative() == !p2.is_absolute());

        key_path p3 = TEXT("\\HKEY_CURRENT_USER\\Test2\\Test3");
        ASSERT_TRUE(!p3.is_absolute());
        ASSERT_TRUE(p3.is_relative() == !p3.is_absolute());

        key_path p4 = TEXT("HKEY_CURRENT_USER\\Test2\\Test3");
        ASSERT_TRUE(p4.is_absolute());
        ASSERT_TRUE(p4.is_relative() == !p4.is_absolute());
    }
}

TEST(KeyPath, Decomposition)
{
    // key_path::root_key()
    {
        key_path p1;
        ASSERT_TRUE(p1.root_key() == (p1.has_root_key() ? key_path(*p1.begin(), p1.key_view()) 
                                                        : key_path(string_type(), p1.key_view())));

        key_path p2 = TEXT("Test");
        ASSERT_TRUE(p2.root_key() == (p2.has_root_key() ? key_path(*p2.begin(), p2.key_view()) 
                                                        : key_path(string_type(), p2.key_view())));

        key_path p3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(p3.root_key() == (p3.has_root_key() ? key_path(*p3.begin(), p3.key_view()) 
                                                        : key_path(string_type(), p3.key_view())));
    }

    // key_path::leaf_key()
    {
        key_path p1;
        ASSERT_TRUE(p1.leaf_key() == (p1.has_leaf_key() ? key_path(*--p1.end(), p1.key_view()) 
                                                        : key_path(string_type(), p1.key_view())));

        key_path p2 = TEXT("Test");
        ASSERT_TRUE(p2.leaf_key() == (p2.has_leaf_key() ? key_path(*--p2.end(), p2.key_view()) 
                                                        : key_path(string_type(), p2.key_view())));

        key_path p3 = TEXT("Test1\\Test2\\Test3");
        ASSERT_TRUE(p3.leaf_key() == (p3.has_leaf_key() ? key_path(*--p3.end(), p3.key_view()) 
                                                        : key_path(string_type(), p3.key_view())));
    }

    // key_path::parent_key()
    {
        key_path p1;
        ASSERT_TRUE(p1.parent_key() == key_path(string_type(), p1.key_view()));

        key_path p2 = TEXT("Test");
        ASSERT_TRUE(p2.parent_key() == key_path(string_type(), p2.key_view()));

        key_path p3 = TEXT("Test1\\Test2\\\\Test3\\");
        ASSERT_TRUE(p3.parent_key() == key_path(TEXT("Test1\\Test2"), p3.key_view()));
    }
}

TEST(KeyPath, Modifiers)
{
    // key_path::append(const Source&)
    {
        // redundant separator case 1
        key_path p1 = TEXT("HKEY_CURRENT_USER\\");
        ASSERT_TRUE(p1.append(TEXT("Test")).key_name() == TEXT("HKEY_CURRENT_USER\\Test"));

        // redundant separator case 2
        key_path p2 = TEXT("");
        key_path p3 = TEXT("\\\\");
        ASSERT_TRUE(p2.append(TEXT("Test")).key_name() == TEXT("Test"));
        ASSERT_TRUE(p3.append(TEXT("Test")).key_name() == TEXT("\\\\Test"));

        // redundant separator case 3
        key_path p4 = TEXT("HKEY_CURRENT_USER");
        ASSERT_TRUE(p4.append(TEXT("")).key_name() == TEXT("HKEY_CURRENT_USER"));

        // redundant separator case 4
        key_path p5 = TEXT("HKEY_CURRENT_USER");
        ASSERT_TRUE(p5.append(TEXT("\\Test")).key_name() == TEXT("HKEY_CURRENT_USER\\Test"));

        // adds a separator
        key_path p6 = TEXT("HKEY_CURRENT_USER");
        ASSERT_TRUE(p6.append(TEXT("Test")).key_name() == TEXT("HKEY_CURRENT_USER\\Test"));
    }

    // key_path::append(const key_path&)
    {
        key_path p1(TEXT("HKEY_CURRENT_USER"), view::view_32bit);
        key_path p2(TEXT("Test1\\Test2\\\\"), view::view_64bit);

        p1.append(p2);
        ASSERT_TRUE(p1.key_name() == TEXT("HKEY_CURRENT_USER\\Test1\\Test2") && p1.key_view() == view::view_64bit);
    }

    // key_path::concat(string_view_type)
    {
        key_path p1, p1_copy = p1;
        ASSERT_TRUE(p1.concat(TEXT("Test")) == key_path(p1_copy.key_name() + TEXT("Test"), p1_copy.key_view()));

        key_path p2 = TEXT("HKEY_CURRENT_USER"), p2_copy = p2;
        ASSERT_TRUE(p2.concat(TEXT("Test")) == key_path(p2_copy.key_name() + TEXT("Test"), p2_copy.key_view()));

        key_path p3 = TEXT("HKEY_CURRENT_USER\\"), p3_copy = p3;
        ASSERT_TRUE(p3.concat(TEXT("Test")) == key_path(p3_copy.key_name() + TEXT("Test"), p3_copy.key_view()));
    }

    // key_path::remove_leaf_key()
    {
        key_path p1 = TEXT("HKEY_CURRENT_USER");
        ASSERT_TRUE(p1.remove_leaf_key().key_name() == TEXT(""));

        key_path p2 = TEXT("HKEY_CURRENT_USER\\");
        ASSERT_TRUE(p2.remove_leaf_key().key_name() == TEXT(""));

        key_path p3 = TEXT("\\HKEY_CURRENT_USER\\");
        ASSERT_TRUE(p3.remove_leaf_key().key_name() == TEXT(""));

        key_path p4 = TEXT("HKEY_CURRENT_USER\\Test");
        ASSERT_TRUE(p4.remove_leaf_key().key_name() == TEXT("HKEY_CURRENT_USER"));

        key_path p5 = TEXT("HKEY_CURRENT_USER\\Test\\\\");
        ASSERT_TRUE(p5.remove_leaf_key().key_name() == TEXT("HKEY_CURRENT_USER"));
    }

    // key_path::replace_leaf_key(string_view_type)
    {
        key_path p1 = TEXT("HKEY_CURRENT_USER"), p1_copy = p1;
        ASSERT_TRUE(p1.replace_leaf_key(TEXT("replacement")) == p1_copy.remove_leaf_key().append(TEXT("replacement")));

        key_path p2 = TEXT("HKEY_CURRENT_USER\\"), p2_copy = p2;
        ASSERT_TRUE(p2.replace_leaf_key(TEXT("replacement")) == p2_copy.remove_leaf_key().append(TEXT("replacement")));

        key_path p3 = TEXT("\\HKEY_CURRENT_USER\\"), p3_copy = p3;
        ASSERT_TRUE(p3.replace_leaf_key(TEXT("replacement")) == p3_copy.remove_leaf_key().append(TEXT("replacement")));

        key_path p4 = TEXT("HKEY_CURRENT_USER\\Test"), p4_copy = p4;
        ASSERT_TRUE(p4.replace_leaf_key(TEXT("replacement")) == p4_copy.remove_leaf_key().append(TEXT("replacement")));

        key_path p5 = TEXT("HKEY_CURRENT_USER\\Test\\\\"), p5_copy = p5;
        ASSERT_TRUE(p5.replace_leaf_key(TEXT("replacement")) == p5_copy.remove_leaf_key().append(TEXT("replacement")));
    }
}

TEST(KeyPath, Swap)
{
    key_path p1(TEXT("HKEY_CURRENT_USER\\Test"), view::view_32bit), p1_copy = p1;
    key_path p2(TEXT("HKEY_LOCAL_MACHINE\\Test"), view::view_64bit), p2_copy = p2;

    swap(p1, p2);
    ASSERT_TRUE(p1 == p2_copy && p2 == p1_copy);
}

TEST(KeyPath, Hash)
{
    key_path p01, p02;
    ASSERT_TRUE(hash_value(p01) == hash_value(p02));

    key_path p03 = TEXT("HKEY_CURRENT_USER\\Test"), p04 = p03;
    ASSERT_TRUE(hash_value(p03) == hash_value(p04));

    key_path p05 = TEXT("HKEY_CURRENT_USER\\Test");
    key_path p06 = TEXT("HKEY_CURRENT_user\\\\Test\\");
    ASSERT_TRUE(hash_value(p05) == hash_value(p06));
}