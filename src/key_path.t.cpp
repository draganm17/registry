#include <algorithm>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key_path.h>
#include <registry/name.h>

using namespace registry;


namespace {

    static const std::pair<const char*, const char*> construction_test_set[]
    {
    //  input name string                                    expected output name string
        { "",                                                "" },
        { "\\",                                              "" },
        { "\\\\",                                            "" },
        { "Test",                                            "Test" },
        { "\\Test",                                          "Test" },
        { "Test\\",                                          "Test" },
        { "Test1\\Test2\\Test3",                             "Test1\\Test2\\Test3" },
        { "\\Test1\\Test2\\Test3",                           "Test1\\Test2\\Test3" },
        { "Test1\\Test2\\Test3\\",                           "Test1\\Test2\\Test3" },
        { "\\\\Test1\\Test2\\\\Test3\\\\",                   "Test1\\Test2\\Test3" },
        { "HKEY_local_machine",                              "HKEY_local_machine" },
        { "HKEY_local_machine\\\\Test1\\Test2\\\\Test3\\\\", "HKEY_local_machine\\Test1\\Test2\\Test3" },
    };

    bool test_name_view_constructors_impl(const std::string& in_name,  view in_view,
                                          const name&        exp_name, view exp_view)
    {
        // key_path(name&&, view view)
        {
            key_path p(name(in_name), in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        // key_path(std::basic_string<name::value_type>&&, view)
        {
            key_path p(name(in_name).value(), in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        // key_path(const Source&, view)
        {
            key_path p(in_name, in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        // key_path(const Source&, const std::locale&, view)
        {
            key_path p(in_name, std::locale(), in_view);
            return p.key_name() == exp_name && p.key_view() == exp_view;
        }

        // key_path(InputIt, InputIt, view)
        {
            key_path p(in_name.begin(), in_name.end(), in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        // key_path(InputIt, InputIt, const std::locale&, view)
        {
            key_path p(in_name.begin(), in_name.end(), std::locale(), in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        return true;
    }

    bool test_name_view_constructors()
    {
        for (auto it =  std::begin(construction_test_set);
                  it != std::end(construction_test_set); ++it)
        {
            if (!test_name_view_constructors_impl(it->first,  view::view_64bit,
                                                  it->second, view::view_64bit))
            {
                return false;
            }
        }
        return true;
    }

    bool test_name_view_assignments_impl(const std::string& in_name,  view in_view,
                                         const name&        exp_name, view exp_view)
    {
        // operator=(name&&)
        {
            key_path p("placeholder");
            p = name(in_name);
            if (p.key_name() != exp_name || p.key_view() != view::view_default)
            {
                return false;
            }
        }

        // operator=(std::basic_string<name::value_type>&&)
        {
            key_path p("placeholder");
            p = name(in_name).value();
            if (p.key_name() != exp_name || p.key_view() != view::view_default)
            {
                return false;
            }
        }

        // operator=(const Source&)
        {
            key_path p("placeholder");
            p = in_name;
            if (p.key_name() != exp_name || p.key_view() != view::view_default)
            {
                return false;
            }
        }

        // assign(name&&, view view)
        {
            key_path p("placeholder");
            p.assign(name(in_name), in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        // assign(std::basic_string<name::value_type>&&, view)
        {
            key_path p("placeholder");
            p.assign(name(in_name).value(), in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        // assign(const Source&, view)
        {
            key_path p("placeholder");
            p.assign(in_name, in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        // assign(const Source&, const std::locale&, view)
        {
            key_path p("placeholder");
            p.assign(in_name, std::locale(), in_view);
            return p.key_name() == exp_name && p.key_view() == exp_view;
        }

        // assign(InputIt, InputIt, view)
        {
            key_path p("placeholder");
            p.assign(in_name.begin(), in_name.end(), in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        // assign(InputIt, InputIt, const std::locale&, view)
        {
            key_path p("placeholder");
            p.assign(in_name.begin(), in_name.end(), std::locale(), in_view);
            if (p.key_name() != exp_name || p.key_view() != exp_view)
            {
                return false;
            }
        }

        return true;
    }

    bool test_name_view_assignments()
    {
        for (auto it =  std::begin(construction_test_set);
                  it != std::end(construction_test_set); ++it)
        {
            if (!test_name_view_assignments_impl(it->first,  view::view_64bit,
                                                 it->second, view::view_64bit))
            {
                return false;
            }
        }
        return true;
    }

} // anonymous namespace


TEST(KeyPath, Construct) 
{
    // default constructor
    {
        key_path p;
        EXPECT_TRUE(p.key_name().empty());
        EXPECT_TRUE(p.key_view() == view::view_default);
    }

    // key_path(view)
    {
        key_path p(view::view_64bit);
        EXPECT_TRUE(p.key_name().empty());
        EXPECT_TRUE(p.key_view() == view::view_64bit);
    }

    // other constructors
    {
        EXPECT_TRUE(test_name_view_constructors());
    }
}

TEST(KeyPath, Assign)
{
    // assign(view)
    {
        key_path p("placeholder");
        p.assign(view::view_64bit);
        EXPECT_TRUE(p.key_name().empty());
        EXPECT_TRUE(p.key_view() == view::view_64bit);
    }

    // other assignments
    {
        EXPECT_TRUE(test_name_view_assignments());
    }
}

TEST(KeyPath, FromKeyId)
{
    static const auto test = [](key_id id, auto&& expected_name)
    {
        const key_path p = key_path::from_key_id(id);
        EXPECT_TRUE(p.key_name() == expected_name);
        EXPECT_TRUE(p.key_view() == view::view_default);
        EXPECT_TRUE(p.root_key_id() == id);
    };

    test(key_id::classes_root,                "HKEY_CLASSES_ROOT");
    test(key_id::current_user,                "HKEY_CURRENT_USER");
    test(key_id::local_machine,               "HKEY_LOCAL_MACHINE");
    test(key_id::users,                       "HKEY_USERS");
    test(key_id::performance_data,            "HKEY_PERFORMANCE_DATA");
    test(key_id::performance_text,            "HKEY_PERFORMANCE_TEXT");
    test(key_id::performance_nlstext,         "HKEY_PERFORMANCE_NLSTEXT");
    test(key_id::current_config,              "HKEY_CURRENT_CONFIG");
    test(key_id::current_user_local_settings, "HKEY_CURRENT_USER_LOCAL_SETTINGS");
    test(key_id::unknown,                     "");
}

TEST(KeyPath, Compare)
{
    EXPECT_TRUE(key_path().compare(key_path()) == 0);

    EXPECT_TRUE(key_path("AAA").compare(key_path("AAA")) == 0);

    EXPECT_TRUE(key_path("AAA").compare(key_path("aAa")) == 0);

    EXPECT_TRUE(key_path("AAA\\BBB").compare(key_path("AAA\\BBB")) == 0);

    EXPECT_TRUE(key_path("AAA\\BBB").compare(key_path("aAa\\bBb")) == 0);

    EXPECT_TRUE(key_path("AAA\\BBB").compare(key_path("AAA\\CCC")) < 0);

    EXPECT_TRUE(key_path("AAA\\BBB").compare(key_path("AAA\\AAA")) > 0);

    EXPECT_TRUE(key_path("AAA\\AAA", view::view_32bit).compare(key_path("AAA\\BBB", view::view_64bit)) > 0);

    EXPECT_TRUE(key_path("AAA\\BBB", view::view_64bit).compare(key_path("AAA\\AAA", view::view_32bit)) < 0);
}

TEST(KeyPath, Iterate)
{
    key_path p01;
    EXPECT_TRUE(p01.begin() == p01.end());

    key_path p02("Test", view::view_32bit);
    auto p02_it = p02.begin(), p02_end = p02.end();
    EXPECT_TRUE(std::distance(p02_it, p02_end) == 1);
    EXPECT_TRUE(*p02_it++ == key_path("Test", p02.key_view()));
    EXPECT_TRUE(p02_it == p02_end);

    key_path p03("Test1\\Test2\\Test3", view::view_64bit);
    auto p03_it = p03.begin(), p03_end = p03.end();
    EXPECT_TRUE(std::distance(p03_it, p03_end) == 3);
    EXPECT_TRUE(*p03_it++ == key_path("Test1", p03.key_view()));
    EXPECT_TRUE(*p03_it++ == key_path("Test2", p03.key_view()));
    EXPECT_TRUE(*p03_it++ == key_path("Test3", p03.key_view()));
    EXPECT_TRUE(p03_it == p03_end);

    key_path p04("Test1\\Test2\\Test3", view::view_32bit);
    auto p04_begin = p04.begin(), p04_it = p04.end();
    EXPECT_TRUE(*--p04_it == key_path("Test3", p04.key_view()));
    EXPECT_TRUE(*--p04_it == key_path("Test2", p04.key_view()));
    EXPECT_TRUE(*--p04_it == key_path("Test1", p04.key_view()));
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

        key_path p2("Test", view::view_32bit);
        EXPECT_TRUE(!p2.has_root_path() &&
                    p2.root_path() == key_path(p2.key_view()));

        key_path p3("Test1\\Test2\\Test3", view::view_64bit);
        EXPECT_TRUE(!p3.has_root_path() &&
                    p3.root_path() == key_path(p3.key_view()));

        key_path p4("Test1\\Test2\\Test3", view::view_64bit);
        EXPECT_TRUE(!p4.has_root_path() &&
                    p4.root_path() == key_path(p4.key_view()));

        key_path p5 = "HKEY_LOCAL_MACHINE";
        EXPECT_TRUE(p5.has_root_path() &&
                    p5.root_path() == *p5.begin());

        key_path p6 = "HKEY_LOCAL_MACHINE\\Test1\\Test2";
        EXPECT_TRUE(p6.has_root_path() &&
                    p6.root_path() == *p6.begin());
    }

    // key_path::root_key_id()
    {
        EXPECT_TRUE(key_path().root_key_id() == key_id::unknown);

        EXPECT_TRUE(key_path("HKEY_CLASSES_root").root_key_id() == key_id::classes_root);

        EXPECT_TRUE(key_path("HKEY_CURRENT_user").root_key_id() == key_id::current_user);

        EXPECT_TRUE(key_path("HKEY_LOCAL_machine").root_key_id() == key_id::local_machine);

        EXPECT_TRUE(key_path("HKEY_users").root_key_id() == key_id::users);

        EXPECT_TRUE(key_path("HKEY_PERFORMANCE_data").root_key_id() == key_id::performance_data);

        EXPECT_TRUE(key_path("HKEY_PERFORMANCE_text").root_key_id() == key_id::performance_text);

        EXPECT_TRUE(key_path("HKEY_PERFORMANCE_nlstext").root_key_id() == key_id::performance_nlstext);

        EXPECT_TRUE(key_path("HKEY_CURRENT_config").root_key_id() == key_id::current_config);

        EXPECT_TRUE(key_path("HKEY_CURRENT_USER_LOCAL_settings").root_key_id() == key_id::current_user_local_settings);
    }

    // key_path::leaf_path()
    // key_path::has_leaf_path()
    {
        key_path p1;
        EXPECT_TRUE(!p1.has_leaf_path() &&
                    p1.root_path() == key_path(p1.key_view()));

        key_path p2("Test", view::view_32bit);
        EXPECT_TRUE(p2.has_leaf_path() &&
                    p2.leaf_path() == *--p2.end());

        key_path p3("Test1\\Test2\\Test3", view::view_64bit);
        EXPECT_TRUE(p3.has_leaf_path() &&
                    p3.leaf_path() == *--p3.end());
    }

    // key_path::parent_path()
    // key_path::has_parent_path()
    {
        key_path p1;
        EXPECT_TRUE(!p1.has_parent_path() &&
                    p1.parent_path() == key_path(p1.key_view()));

        key_path p2("Test", view::view_32bit);
        EXPECT_TRUE(!p2.has_parent_path() &&
                    p2.parent_path() == key_path(p2.key_view()));

        key_path p3("Test1\\Test2", view::view_32bit);
        EXPECT_TRUE(p3.has_parent_path() &&
                    p3.parent_path() == *p3.begin());

        key_path p4 = "Test1\\Test2\\Test3";
        EXPECT_TRUE(p4.has_parent_path() &&
                    p4.parent_path() == (*p4.begin() / *++p4.begin())
        );
    }

    // key_path::is_absolute()
    // key_path::is_relative()
    {
        key_path p1;
        EXPECT_TRUE(!p1.is_absolute());
        EXPECT_TRUE(p1.is_relative() == !p1.is_absolute());

        key_path p2 = "Test1\\Test2\\Test3";
        EXPECT_TRUE(!p2.is_absolute());
        EXPECT_TRUE(p2.is_relative() == !p2.is_absolute());

        key_path p3 = "HKEY_CURRENT_USER\\Test2\\Test3";
        EXPECT_TRUE(p3.is_absolute());
        EXPECT_TRUE(p3.is_relative() == !p3.is_absolute());
    }
}

TEST(KeyPath, Modifiers)
{
    // key_path::append(const Source&)
    // key_path::operator+=(const Source&)
    {
        // path to path
        {
            // TODO: ...
        }

        // string to path
        {
            // TODO: ...
        }

        // redundant separator case 1
        //key_path p1 = "HKEY_CURRENT_USER\\";
        //EXPECT_TRUE(p1.append("Test").key_name() == "HKEY_CURRENT_USER\\Test");

        // redundant separator case 2
        //key_path p2 = "";
        //key_path p3 = "\\\\";
        //EXPECT_TRUE(p2.append("Test").key_name() == "Test");
        //EXPECT_TRUE(p3.append("Test").key_name() == "\\\\Test");

        // redundant separator case 3
        //key_path p4 = "HKEY_CURRENT_USER";
        //EXPECT_TRUE(p4.append("").key_name() == "HKEY_CURRENT_USER");

        // redundant separator case 4
        //key_path p5 = "HKEY_CURRENT_USER";
        //EXPECT_TRUE(p5.append("\\Test").key_name() == "HKEY_CURRENT_USER\\Test");

        // adds a separator
        //key_path p6 = "HKEY_CURRENT_USER";
        //EXPECT_TRUE(p6.append("Test").key_name() == "HKEY_CURRENT_USER\\Test");
    }

    // key_path::concat(const Source&)
    // key_path::operator+=(const Source&)
    {
        // TODO: ...

        //key_path p1, p1_copy = p1;
        ///EXPECT_TRUE(p1.concat("Test") == key_path(p1_copy.key_name() + "Test", p1_copy.key_view()));

        //key_path p2 = "HKEY_CURRENT_USER", p2_copy = p2;
        //EXPECT_TRUE(p2.concat("Test") == key_path(p2_copy.key_name() + "Test", p2_copy.key_view()));

        //key_path p3 = "HKEY_CURRENT_USER\\", p3_copy = p3;
        //EXPECT_TRUE(p3.concat("Test") == key_path(p3_copy.key_name() + "Test", p3_copy.key_view()));
    }

    // key_path::remove_leaf_path()
    {
        key_path p1(view::view_32bit);
        EXPECT_TRUE(p1.remove_leaf_path() == key_path(view::view_32bit));

        key_path p2("HKEY_CURRENT_USER", view::view_64bit);
        EXPECT_TRUE(p2.remove_leaf_path() == key_path(view::view_64bit));

        key_path p4("HKEY_CURRENT_USER\\Test1\\Test2", view::view_64bit);
        EXPECT_TRUE(p4.remove_leaf_path() == key_path("HKEY_CURRENT_USER\\Test1", view::view_64bit));
    }

    // key_path::replace_leaf_path(const Source&)
    {
        // TODO: ...

        //key_path p1 = "HKEY_CURRENT_USER", p1_copy = p1;
        //EXPECT_TRUE(p1.replace_leaf_path("replacement") == p1_copy.remove_leaf_path().append("replacement"));

        //key_path p2 = "HKEY_CURRENT_USER\\", p2_copy = p2;
        //EXPECT_TRUE(p2.replace_leaf_path("replacement") == p2_copy.remove_leaf_path().append("replacement"));

        //key_path p3 = "\\HKEY_CURRENT_USER\\", p3_copy = p3;
        //EXPECT_TRUE(p3.replace_leaf_path("replacement") == p3_copy.remove_leaf_path().append("replacement"));

        //key_path p4 = "HKEY_CURRENT_USER\\Test", p4_copy = p4;
        //EXPECT_TRUE(p4.replace_leaf_path("replacement") == p4_copy.remove_leaf_path().append("replacement"));

        //key_path p5 = "HKEY_CURRENT_USER\\Test\\\\", p5_copy = p5;
        //EXPECT_TRUE(p5.replace_leaf_path("replacement") == p5_copy.remove_leaf_path().append("replacement"));
    }
}

TEST(KeyPath, Swap)
{
    key_path p1("HKEY_CURRENT_USER\\Test", view::view_32bit), p1_copy = p1;
    key_path p2("HKEY_LOCAL_MACHINE\\Test", view::view_64bit), p2_copy = p2;

    swap(p1, p2);
    EXPECT_TRUE(p1 == p2_copy && p2 == p1_copy);
}

TEST(KeyPath, Hash)
{
    std::hash<key_path> hasher;

    key_path p01, p02;
    EXPECT_TRUE(hasher(p01) == hasher(p02));

    key_path p03 = "HKEY_CURRENT_USER\\Test", p04 = p03;
    EXPECT_TRUE(hasher(p03) == hasher(p04));

    key_path p05 = "HKEY_CURRENT_USER\\Test";
    key_path p06 = "HKEY_CURRENT_user\\\\Test\\";
    EXPECT_TRUE(hasher(p05) == hasher(p06));
}