#include <string>
#include <string_view>
#include <type_traits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/details/string_codec.h>

using namespace registry;
using namespace registry::details;

namespace {



}


TEST(Details, DeduceEncoding)
{
    EXPECT_TRUE((std::is_same<deduce_encoding_t<char>, system_narrow_encoding>::value));
    EXPECT_TRUE((std::is_same<deduce_encoding_t<wchar_t>, system_wide_encoding>::value));

    EXPECT_TRUE((std::is_same<deduce_encoding_t<const char*>, system_narrow_encoding>::value));
    EXPECT_TRUE((std::is_same<deduce_encoding_t<const wchar_t*>, system_wide_encoding>::value));
}

TEST(Details, IsEncodable_IsDecodable)
{
    EXPECT_TRUE((is_encodable<const wchar_t*, system_narrow_encoding>::value));
    EXPECT_TRUE((is_encodable<const wchar_t*, system_wide_encoding>::value));

    EXPECT_TRUE((is_decodable<const char*>::value));
    EXPECT_TRUE((is_decodable<const char*, system_narrow_encoding>::value));

    EXPECT_TRUE((is_decodable<const wchar_t*>::value));
    EXPECT_TRUE((is_decodable<const wchar_t*, system_wide_encoding>::value));

    EXPECT_TRUE((is_decodable<const char16_t*>::value));
}