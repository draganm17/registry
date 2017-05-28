#include <string>
#include <string_view>
#include <type_traits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/details/string_codec.h>

using namespace registry::details::encoding;

namespace {

    struct not_an_encoding { };

    struct my_valid_deduction_policy
    {
        template<typename CharT>
        using encoding_type = default_deduction_policy::encoding_type<CharT>;
    };

    struct my_invalid_deduction_policy_1 { };

    struct my_invalid_deduction_policy_2
    {
        template <typename T>
        using encoding_type = not_an_encoding;
    };
}


TEST(Encoding, IsEncoding)
{
    EXPECT_TRUE(is_encoding<narrow_encoding>::value);
    EXPECT_TRUE(is_encoding<wide_encoding>::value);
    EXPECT_FALSE(is_encoding<not_an_encoding>::value);
}

TEST(Encoding, DefaultDeductionPolicy)
{
    EXPECT_TRUE((std::is_same<typename default_deduction_policy::encoding_type<char>, narrow_encoding>::value));
    EXPECT_TRUE((std::is_same<typename default_deduction_policy::encoding_type<wchar_t>, wide_encoding>::value));
}

TEST(Encoding, IsDeducible)
{
    // for a character
    EXPECT_TRUE(is_deducible<char>::value);
    EXPECT_TRUE(is_deducible<wchar_t>::value);

    // for a string
    EXPECT_TRUE(is_deducible<const char*>::value);
    EXPECT_TRUE(is_deducible<const wchar_t*>::value);

    // for neither a string nor a character
    EXPECT_FALSE(is_deducible<int>::value);

    // with an user-provided deduction policy
    EXPECT_TRUE((is_deducible<char, my_valid_deduction_policy>::value));

    // with an invalid user-provided deduction policy
    EXPECT_FALSE((is_deducible<char, my_invalid_deduction_policy_1>::value));
    EXPECT_FALSE((is_deducible<char, my_invalid_deduction_policy_2>::value));
}

TEST(Encoding, IsDeducibleTo)
{
    // for a character
    EXPECT_TRUE((is_deducible_to<char, narrow_encoding>::value));
    EXPECT_TRUE((is_deducible_to<wchar_t, wide_encoding>::value));

    // for a string
    EXPECT_TRUE((is_deducible_to<const char*, narrow_encoding>::value));
    EXPECT_TRUE((is_deducible_to<const wchar_t*, wide_encoding>::value));

    // for neither a string nor a character
    EXPECT_FALSE((is_deducible_to<int, narrow_encoding>::value));
    EXPECT_FALSE((is_deducible_to<int, wide_encoding>::value));

    // with an user-provided deduction policy
    EXPECT_TRUE((is_deducible_to<char, narrow_encoding, my_valid_deduction_policy>::value));

    // with an invalid user-provided deduction policy
    EXPECT_FALSE((is_deducible<char, narrow_encoding, my_invalid_deduction_policy_1>::value));
    EXPECT_FALSE((is_deducible<char, wide_encoding, my_invalid_deduction_policy_1>::value));
    EXPECT_FALSE((is_deducible<char, narrow_encoding, my_invalid_deduction_policy_2>::value));
    EXPECT_FALSE((is_deducible<char, wide_encoding, my_invalid_deduction_policy_2>::value));
}

TEST(Encoding, Deduce)
{
    // for a character
    EXPECT_TRUE((std::is_same<deduce_t<char>, narrow_encoding>::value));
    EXPECT_TRUE((std::is_same<deduce_t<wchar_t>, wide_encoding>::value));

    // for a string
    EXPECT_TRUE((std::is_same<deduce_t<const char*>, narrow_encoding>::value));
    EXPECT_TRUE((std::is_same<deduce_t<const wchar_t*>, wide_encoding>::value));

    // with an user-provided deduction policy
    EXPECT_TRUE((std::is_same<deduce_t<char, my_valid_deduction_policy>, narrow_encoding>::value));
}

TEST(Encoding, NarrowCodec)
{
    codec<narrow_encoding> codec;
    const char    enc_str_ref[] =  "Привет Мир!";
    const wchar_t dec_str_ref[] = L"Привет Мир!";

    auto enc_str = codec.encode(std::begin(dec_str_ref), std::end(dec_str_ref) - 1, std::locale("ru-RU"));
    EXPECT_TRUE(enc_str == enc_str_ref);

    auto dec_str = codec.decode(std::begin(enc_str_ref), std::end(enc_str_ref) - 1, std::locale("ru-RU"));
    EXPECT_TRUE(dec_str == dec_str_ref);
}

TEST(Encoding, WideCodec)
{
    codec<wide_encoding> codec;
    const wchar_t enc_str_ref[] = L"Привет Мир!";
    const wchar_t dec_str_ref[] = L"Привет Мир!";

    auto enc_str = codec.encode(std::begin(dec_str_ref), std::end(dec_str_ref) - 1, std::locale("ru-RU"));
    EXPECT_TRUE(enc_str == enc_str_ref);

    auto dec_str = codec.decode(std::begin(enc_str_ref), std::end(enc_str_ref) - 1, std::locale("ru-RU"));
    EXPECT_TRUE(dec_str == dec_str_ref);
}