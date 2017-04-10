#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/value_iterator.h>

using namespace registry;

TEST(ValueIterator, Construct)
{
    // default constructor
    {
        value_iterator it;
        assert(it == value_iterator());
    }

    // value_iterator::value_iterator(const key&)
    // value_iterator::value_iterator(const key&, std::error_code&)
    {
        std::error_code ec;

        value_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"));
        value_iterator it2(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"), ec);
        EXPECT_TRUE(it1 == value_iterator());
        EXPECT_TRUE(it2 == value_iterator() && !ec);

        value_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
        value_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), ec);
        EXPECT_TRUE(it3 != value_iterator());
        EXPECT_TRUE(it4 != value_iterator() && !ec);
    }

    // value_iterator::value_iterator(const key_handle&)
    // value_iterator::value_iterator(const key_handle&, std::error_code&)
    {
        std::error_code ec;

        value_iterator it1(key_handle());
        value_iterator it2(key_handle(), ec);
        //EXPECT_TRUE(it1 == value_iterator());
        EXPECT_TRUE(it2 == value_iterator() && !ec);

        //value_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
        //value_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), ec);
        //EXPECT_TRUE(it3 != value_iterator());
        //EXPECT_TRUE(it4 != value_iterator() && !ec);
    }
}