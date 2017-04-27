#include <iterator>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key_path.h>

using namespace registry;


TEST(KeyPath, Construct) 
{
    // default constructor
    {
        key_path p;
        EXPECT_TRUE(p.key_name().empty());
        EXPECT_TRUE(p.key_view() == view::view_default);
    }

    // construct from view
    {
        key_path p(view::view_64bit);
        EXPECT_TRUE(p.key_name().empty());
        EXPECT_TRUE(p.key_view() == view::view_64bit);
    }

    // construct from name an view
    {
        key_path p00(TEXT("HKEY_CURRENT_user\\Test"), view::view_32bit);
        EXPECT_TRUE(p00.key_name() == TEXT("HKEY_CURRENT_user\\Test"));
        EXPECT_TRUE(p00.key_view() == view::view_32bit);

        key_path p01;
        EXPECT_TRUE(p01.key_name().empty());

        key_path p02 = TEXT("\\");
        EXPECT_TRUE(p02.key_name().empty());

        key_path p03 = TEXT("\\\\");
        EXPECT_TRUE(p03.key_name().empty());

        key_path p04 = TEXT("Test");
        EXPECT_TRUE(p04.key_name() == TEXT("Test"));

        key_path p05 = TEXT("\\Test");
        EXPECT_TRUE(p05.key_name() == TEXT("Test"));

        key_path p06 = TEXT("Test\\");
        EXPECT_TRUE(p06.key_name() == TEXT("Test"));

        key_path p07 = TEXT("\\\\Test\\\\");
        EXPECT_TRUE(p07.key_name() == TEXT("Test"));

        key_path p08 = TEXT("Test1\\Test2\\Test3");
        EXPECT_TRUE(p08.key_name() == TEXT("Test1\\Test2\\Test3"));

        key_path p09 = TEXT("\\Test1\\Test2\\Test3");
        EXPECT_TRUE(p09.key_name() == TEXT("Test1\\Test2\\Test3"));

        key_path p10 = TEXT("Test1\\Test2\\Test3\\");
        EXPECT_TRUE(p10.key_name() == TEXT("Test1\\Test2\\Test3"));

        key_path p11 = TEXT("\\\\Test1\\Test2\\\\Test3\\\\");
        EXPECT_TRUE(p11.key_name() == TEXT("Test1\\Test2\\Test3"));

        // test construction from not null-terminating strings
        //
        key_path p12 = string_view_type();
        EXPECT_TRUE(p12.key_name().empty());
        //
        wchar_t buf2[] = { TEXT('a'), TEXT('b') };
        key_path p14 = string_view_type(buf2, _countof(buf2));
        EXPECT_TRUE(p14.key_name() == TEXT("ab"));
        //
        wchar_t buf3[] = { TEXT('a'), TEXT('\\'), TEXT('b') };
        key_path p15 = string_view_type(buf3, _countof(buf3));
        EXPECT_TRUE(p15.key_name() == TEXT("a\\b"));
    }

    // test implicit from-string construction
    {
        key_path p = TEXT("HKEY_CURRENT_user\\Test");
        EXPECT_TRUE(p.key_name() == TEXT("HKEY_CURRENT_user\\Test"));
        EXPECT_TRUE(p.key_view() == view::view_default);
    }
}

TEST(KeyPath, Assign)
{
    key_path p1(TEXT("Test1"), view::view_32bit);
    key_path p2(TEXT("Test1\\Test2\\Test3"), view::view_64bit);

    EXPECT_TRUE(p1.assign(p2.key_name(), p2.key_view()) == p2);
}

TEST(KeyPath, FromKeyId)
{
    static const auto test = [](key_id id, string_view_type expected_name)
    {
        const key_path p = key_path::from_key_id(id);
        EXPECT_TRUE(p.key_name() == expected_name);
        EXPECT_TRUE(p.key_view() == view::view_default);
        EXPECT_TRUE(p.root_key_id() == id);
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
    EXPECT_TRUE(key_path().compare(key_path()) == 0);

    EXPECT_TRUE(key_path(TEXT("AAA")).compare(key_path(TEXT("AAA"))) == 0);

    EXPECT_TRUE(key_path(TEXT("AAA\\BBB")).compare(key_path(TEXT("AAA\\BBB"))) == 0);

    EXPECT_TRUE(key_path(TEXT("AAA\\BBB")).compare(key_path(TEXT("aAa\\\\bBb\\"))) == 0);

    EXPECT_TRUE(key_path(TEXT("AAA\\BBB")).compare(key_path(TEXT("AAA\\CCC"))) < 0);

    EXPECT_TRUE(key_path(TEXT("AAA\\BBB")).compare(key_path(TEXT("AAA\\AAA"))) > 0);

    EXPECT_TRUE(key_path(TEXT("AAA\\AAA"), view::view_32bit).compare(key_path(TEXT("AAA\\BBB"), view::view_64bit)) > 0);

    EXPECT_TRUE(key_path(TEXT("AAA\\BBB"), view::view_64bit).compare(key_path(TEXT("AAA\\AAA"), view::view_32bit)) < 0);
}

TEST(KeyPath, Iterate)
{
    key_path p01;
    EXPECT_TRUE(p01.begin() == p01.end());

    key_path p02(TEXT("Test"), view::view_32bit);
    auto p02_it = p02.begin(), p02_end = p02.end();
    EXPECT_TRUE(std::distance(p02_it, p02_end) == 1);
    EXPECT_TRUE(*p02_it++ == key_path(TEXT("Test"), p02.key_view()));
    EXPECT_TRUE(p02_it == p02_end);

    key_path p03(TEXT("Test1\\Test2\\Test3"), view::view_64bit);
    auto p03_it = p03.begin(), p03_end = p03.end();
    EXPECT_TRUE(std::distance(p03_it, p03_end) == 3);
    EXPECT_TRUE(*p03_it++ == key_path(TEXT("Test1"), p03.key_view()));
    EXPECT_TRUE(*p03_it++ == key_path(TEXT("Test2"), p03.key_view()));
    EXPECT_TRUE(*p03_it++ == key_path(TEXT("Test3"), p03.key_view()));
    EXPECT_TRUE(p03_it == p03_end);

    key_path p04(TEXT("Test1\\Test2\\Test3"), view::view_32bit);
    auto p04_begin = p04.begin(), p04_it = p04.end();
    EXPECT_TRUE(*--p04_it == key_path(TEXT("Test3"), p04.key_view()));
    EXPECT_TRUE(*--p04_it == key_path(TEXT("Test2"), p04.key_view()));
    EXPECT_TRUE(*--p04_it == key_path(TEXT("Test1"), p04.key_view()));
    EXPECT_TRUE(p04_it == p04_begin);
}

TEST(KeyPath, QueriesAndDecomposition)
{
    // key_path::root_path()
    // key_path::has_root_path()
    {
        key_path p1;
        EXPECT_TRUE(!p1.has_root_path() &&
                    p1.root_path() == key_path(p1.key_view()));

        key_path p2(TEXT("Test"), view::view_32bit);
        EXPECT_TRUE(!p2.has_root_path() &&
                    p2.root_path() == key_path(p2.key_view()));

        key_path p3(TEXT("Test1\\Test2\\Test3"), view::view_64bit);
        EXPECT_TRUE(!p3.has_root_path() &&
                    p3.root_path() == key_path(p3.key_view()));

        key_path p4(TEXT("Test1\\Test2\\Test3"), view::view_64bit);
        EXPECT_TRUE(!p4.has_root_path() &&
                    p4.root_path() == key_path(p4.key_view()));

        key_path p5 = TEXT("HKEY_LOCAL_MACHINE");
        EXPECT_TRUE(p5.has_root_path() &&
                    p5.root_path() == *p5.begin());

        key_path p6 = TEXT("HKEY_LOCAL_MACHINE\\Test1\\Test2");
        EXPECT_TRUE(p6.has_root_path() &&
                    p6.root_path() == *p6.begin());
    }

    // key_path::root_key_id()
    {
        EXPECT_TRUE(key_path().root_key_id() == key_id::unknown);

        EXPECT_TRUE(key_path(TEXT("HKEY_CLASSES_root")).root_key_id() == key_id::classes_root);

        EXPECT_TRUE(key_path(TEXT("HKEY_CURRENT_user")).root_key_id() == key_id::current_user);

        EXPECT_TRUE(key_path(TEXT("HKEY_LOCAL_machine")).root_key_id() == key_id::local_machine);

        EXPECT_TRUE(key_path(TEXT("HKEY_users")).root_key_id() == key_id::users);

        EXPECT_TRUE(key_path(TEXT("HKEY_PERFORMANCE_data")).root_key_id() == key_id::performance_data);

        EXPECT_TRUE(key_path(TEXT("HKEY_PERFORMANCE_text")).root_key_id() == key_id::performance_text);

        EXPECT_TRUE(key_path(TEXT("HKEY_PERFORMANCE_nlstext")).root_key_id() == key_id::performance_nlstext);

        EXPECT_TRUE(key_path(TEXT("HKEY_CURRENT_config")).root_key_id() == key_id::current_config);

        EXPECT_TRUE(key_path(TEXT("HKEY_CURRENT_USER_LOCAL_settings")).root_key_id() == key_id::current_user_local_settings);
    }

    // key_path::leaf_path()
    // key_path::has_leaf_path()
    {
        key_path p1;
        EXPECT_TRUE(!p1.has_leaf_path() &&
                    p1.root_path() == key_path(p1.key_view()));

        key_path p2(TEXT("Test"), view::view_32bit);
        EXPECT_TRUE(p2.has_leaf_path() &&
                    p2.leaf_path() == *--p2.end());

        key_path p3(TEXT("Test1\\Test2\\Test3"), view::view_64bit);
        EXPECT_TRUE(p3.has_leaf_path() &&
                    p3.leaf_path() == *--p3.end());
    }

    // key_path::parent_path()
    // key_path::has_parent_path()
    {
        key_path p1;
        EXPECT_TRUE(!p1.has_parent_path() &&
                    p1.parent_path() == key_path(p1.key_view()));

        key_path p2(TEXT("Test"), view::view_32bit);
        EXPECT_TRUE(!p2.has_parent_path() &&
                    p2.parent_path() == key_path(p2.key_view()));

        key_path p3 = TEXT("Test1\\Test2\\Test3");
        EXPECT_TRUE(p3.has_parent_path() &&
                    p3.parent_path() == (*p3.begin() / *++p3.begin()));
    }

    // key_path::is_absolute()
    // key_path::is_relative()
    {
        key_path p1;
        EXPECT_TRUE(!p1.is_absolute());
        EXPECT_TRUE(p1.is_relative() == !p1.is_absolute());

        key_path p2 = TEXT("Test1\\Test2\\Test3");
        EXPECT_TRUE(!p2.is_absolute());
        EXPECT_TRUE(p2.is_relative() == !p2.is_absolute());

        key_path p3 = TEXT("HKEY_CURRENT_USER\\Test2\\Test3");
        EXPECT_TRUE(p3.is_absolute());
        EXPECT_TRUE(p3.is_relative() == !p3.is_absolute());
    }
}

TEST(KeyPath, Modifiers)
{
    // key_path::append(const Source&)
    {
        // redundant separator case 1
        key_path p1 = TEXT("HKEY_CURRENT_USER\\");
        EXPECT_TRUE(p1.append(TEXT("Test")).key_name() == TEXT("HKEY_CURRENT_USER\\Test"));

        // redundant separator case 2
        key_path p2 = TEXT("");
        key_path p3 = TEXT("\\\\");
        EXPECT_TRUE(p2.append(TEXT("Test")).key_name() == TEXT("Test"));
        EXPECT_TRUE(p3.append(TEXT("Test")).key_name() == TEXT("\\\\Test"));

        // redundant separator case 3
        key_path p4 = TEXT("HKEY_CURRENT_USER");
        EXPECT_TRUE(p4.append(TEXT("")).key_name() == TEXT("HKEY_CURRENT_USER"));

        // redundant separator case 4
        key_path p5 = TEXT("HKEY_CURRENT_USER");
        EXPECT_TRUE(p5.append(TEXT("\\Test")).key_name() == TEXT("HKEY_CURRENT_USER\\Test"));

        // adds a separator
        key_path p6 = TEXT("HKEY_CURRENT_USER");
        EXPECT_TRUE(p6.append(TEXT("Test")).key_name() == TEXT("HKEY_CURRENT_USER\\Test"));
    }

    // key_path::append(const key_path&)
    {
        key_path p1(TEXT("HKEY_CURRENT_USER"), view::view_32bit);
        key_path p2(TEXT("Test1\\Test2\\\\"), view::view_64bit);

        p1.append(p2);
        EXPECT_TRUE(p1.key_name() == TEXT("HKEY_CURRENT_USER\\Test1\\Test2") && p1.key_view() == view::view_64bit);
    }

    // key_path::concat(string_view_type)
    {
        key_path p1, p1_copy = p1;
        EXPECT_TRUE(p1.concat(TEXT("Test")) == key_path(p1_copy.key_name() + TEXT("Test"), p1_copy.key_view()));

        key_path p2 = TEXT("HKEY_CURRENT_USER"), p2_copy = p2;
        EXPECT_TRUE(p2.concat(TEXT("Test")) == key_path(p2_copy.key_name() + TEXT("Test"), p2_copy.key_view()));

        key_path p3 = TEXT("HKEY_CURRENT_USER\\"), p3_copy = p3;
        EXPECT_TRUE(p3.concat(TEXT("Test")) == key_path(p3_copy.key_name() + TEXT("Test"), p3_copy.key_view()));
    }

    // key_path::remove_leaf_path()
    {
        key_path p1 = TEXT("HKEY_CURRENT_USER");
        EXPECT_TRUE(p1.remove_leaf_path().key_name() == TEXT(""));

        key_path p2 = TEXT("HKEY_CURRENT_USER\\");
        EXPECT_TRUE(p2.remove_leaf_path().key_name() == TEXT(""));

        key_path p3 = TEXT("\\HKEY_CURRENT_USER\\");
        EXPECT_TRUE(p3.remove_leaf_path().key_name() == TEXT(""));

        key_path p4 = TEXT("HKEY_CURRENT_USER\\Test");
        EXPECT_TRUE(p4.remove_leaf_path().key_name() == TEXT("HKEY_CURRENT_USER"));

        key_path p5 = TEXT("HKEY_CURRENT_USER\\Test\\\\");
        EXPECT_TRUE(p5.remove_leaf_path().key_name() == TEXT("HKEY_CURRENT_USER"));
    }

    // key_path::replace_leaf_path(string_view_type)
    {
        key_path p1 = TEXT("HKEY_CURRENT_USER"), p1_copy = p1;
        EXPECT_TRUE(p1.replace_leaf_path(TEXT("replacement")) == p1_copy.remove_leaf_path().append(TEXT("replacement")));

        key_path p2 = TEXT("HKEY_CURRENT_USER\\"), p2_copy = p2;
        EXPECT_TRUE(p2.replace_leaf_path(TEXT("replacement")) == p2_copy.remove_leaf_path().append(TEXT("replacement")));

        key_path p3 = TEXT("\\HKEY_CURRENT_USER\\"), p3_copy = p3;
        EXPECT_TRUE(p3.replace_leaf_path(TEXT("replacement")) == p3_copy.remove_leaf_path().append(TEXT("replacement")));

        key_path p4 = TEXT("HKEY_CURRENT_USER\\Test"), p4_copy = p4;
        EXPECT_TRUE(p4.replace_leaf_path(TEXT("replacement")) == p4_copy.remove_leaf_path().append(TEXT("replacement")));

        key_path p5 = TEXT("HKEY_CURRENT_USER\\Test\\\\"), p5_copy = p5;
        EXPECT_TRUE(p5.replace_leaf_path(TEXT("replacement")) == p5_copy.remove_leaf_path().append(TEXT("replacement")));
    }
}

TEST(KeyPath, Swap)
{
    key_path p1(TEXT("HKEY_CURRENT_USER\\Test"), view::view_32bit), p1_copy = p1;
    key_path p2(TEXT("HKEY_LOCAL_MACHINE\\Test"), view::view_64bit), p2_copy = p2;

    swap(p1, p2);
    EXPECT_TRUE(p1 == p2_copy && p2 == p1_copy);
}

TEST(KeyPath, Hash)
{
    key_path p01, p02;
    EXPECT_TRUE(hash_value(p01) == hash_value(p02));

    key_path p03 = TEXT("HKEY_CURRENT_USER\\Test"), p04 = p03;
    EXPECT_TRUE(hash_value(p03) == hash_value(p04));

    key_path p05 = TEXT("HKEY_CURRENT_USER\\Test");
    key_path p06 = TEXT("HKEY_CURRENT_user\\\\Test\\");
    EXPECT_TRUE(hash_value(p05) == hash_value(p06));
}