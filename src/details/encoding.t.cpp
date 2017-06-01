#include <string>
#include <string_view>
#include <type_traits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/details/encoding.h>

using namespace registry::details::encoding;

namespace {

    struct not_encoding { };

    struct invalid_deduction_policy { };

    template <typename CharT, typename DP = default_deduction_policy>
    inline bool test_is_encoded_character()
    {
        return is_encoded_character<               CharT, DP>::value &&
               is_encoded_character<const          CharT, DP>::value &&
               is_encoded_character<      volatile CharT, DP>::value &&
               is_encoded_character<const volatile CharT, DP>::value;
    }

    template <typename CharT, typename DP = default_deduction_policy>
    inline bool test_is_encoded_string_char_ptr()
    {
        return is_encoded_string< CharT                *               , DP>::value &&
               is_encoded_string< CharT const          *               , DP>::value &&
               is_encoded_string< CharT                * const         , DP>::value &&
               is_encoded_string< CharT const          * const         , DP>::value &&
               is_encoded_string< CharT                *               , DP>::value &&
               is_encoded_string< CharT       volatile *               , DP>::value &&
               is_encoded_string< CharT                *       volatile, DP>::value &&
               is_encoded_string< CharT       volatile *       volatile, DP>::value &&
               is_encoded_string< CharT                *               , DP>::value &&
               is_encoded_string< CharT const volatile *               , DP>::value &&
               is_encoded_string< CharT                * const volatile, DP>::value &&
               is_encoded_string< CharT const volatile * const volatile, DP>::value;
    }

    template <typename CharT, typename DP = default_deduction_policy>
    inline bool test_is_encoded_string_char_arr()
    {
        return is_encoded_string<                CharT[5], DP>::value &&
               is_encoded_string< const          CharT[5], DP>::value &&
               is_encoded_string<       volatile CharT[5], DP>::value &&
               is_encoded_string< const volatile CharT[5], DP>::value;
    }

    template <typename CharT, typename DP = default_deduction_policy>
    inline bool test_is_encoded_string_std_basic_string()
    {
        return is_encoded_string<                std::basic_string<CharT>, DP>::value &&
               is_encoded_string< const          std::basic_string<CharT>, DP>::value &&
               is_encoded_string<       volatile std::basic_string<CharT>, DP>::value &&
               is_encoded_string< const volatile std::basic_string<CharT>, DP>::value;
    }

    template <typename CharT, typename DP = default_deduction_policy>
    inline bool test_is_encoded_string_std_basic_string_view()
    {
        return is_encoded_string<                std::basic_string_view<               CharT>, DP>::value &&
               is_encoded_string< const          std::basic_string_view<               CharT>, DP>::value &&
               is_encoded_string<       volatile std::basic_string_view<               CharT>, DP>::value &&
               is_encoded_string< const volatile std::basic_string_view<               CharT>, DP>::value &&
               is_encoded_string<                std::basic_string_view<const          CharT>, DP>::value &&
               is_encoded_string<                std::basic_string_view<      volatile CharT>, DP>::value &&
               is_encoded_string<                std::basic_string_view<const volatile CharT>, DP>::value;
    }

    template <typename CharT>
    bool test_string_traits_char_ptr()
    {
        CharT str[] = { CharT('T'), CharT('E'), CharT('S'), CharT('T'), CharT('\0') };

        return string_traits< CharT                *>::data(str) == &str[0] &&
               string_traits< CharT const          *>::data(str) == &str[0] &&
               string_traits< CharT       volatile *>::data(str) == &str[0] &&
               string_traits< CharT const volatile *>::data(str) == &str[0] &&

               string_traits< CharT                *>::size(str) == 4 &&
               string_traits< CharT const          *>::size(str) == 4 &&
               string_traits< CharT       volatile *>::size(str) == 4 &&
               string_traits< CharT const volatile *>::size(str) == 4 &&

               std::is_same<string_traits< CharT                *>::char_type,                CharT>::value &&
               std::is_same<string_traits< CharT const          *>::char_type, const          CharT>::value &&
               std::is_same<string_traits< CharT       volatile *>::char_type,       volatile CharT>::value &&
               std::is_same<string_traits< CharT const volatile *>::char_type, const volatile CharT>::value;
    }

    template <typename CharT>
    bool test_string_traits_char_arr()
    {
        CharT str[] = { CharT('T'), CharT('E'), CharT('S'), CharT('T'), CharT('\0') };

        return string_traits<                CharT[5] >::data(str) == &str[0] &&
               string_traits< const          CharT[5] >::data(str) == &str[0] &&
               string_traits<       volatile CharT[5] >::data(str) == &str[0] &&
               string_traits< const volatile CharT[5] >::data(str) == &str[0] &&

               string_traits<                CharT[5] >::size(str) == 4 &&
               string_traits< const          CharT[5] >::size(str) == 4 &&
               string_traits<       volatile CharT[5] >::size(str) == 4 &&
               string_traits< const volatile CharT[5] >::size(str) == 4 &&

               std::is_same<string_traits<                CharT[5] >::char_type,                CharT>::value &&
               std::is_same<string_traits< const          CharT[5] >::char_type, const          CharT>::value &&
               std::is_same<string_traits<       volatile CharT[5] >::char_type,       volatile CharT>::value &&
               std::is_same<string_traits< const volatile CharT[5] >::char_type, const volatile CharT>::value;
    }

    template <typename CharT>
    bool test_string_traits_std_basic_string()
    {
        std::basic_string<CharT> str(4, CharT('x'));

        return string_traits<std::basic_string<CharT> >::data(str) == &str[0]  &&
               string_traits<std::basic_string<CharT> >::size(str) == 4        &&
               std::is_same<typename string_traits<std::basic_string<CharT>>::char_type, CharT>::value;
    }

    template <typename CharT>
    bool test_string_traits_std_basic_string_view()
    {
        std::basic_string<CharT> s(4, CharT('x'));
        std::basic_string_view<CharT> str = s;

        return string_traits<std::basic_string_view<               CharT> >::data(str) == &str[0] &&
             //string_traits<std::basic_string_view<const          CharT> >::data(str) == &str[0] &&
             //string_traits<std::basic_string_view<      volatile CharT> >::data(str) == &str[0] &&
             //string_traits<std::basic_string_view<const volatile CharT> >::data(str) == &str[0] &&

               string_traits<std::basic_string_view<               CharT> >::size(str) == 4 &&
             //string_traits<std::basic_string_view<const          CharT> >::size(str) == 4 &&
             //string_traits<std::basic_string_view<      volatile CharT> >::size(str) == 4 &&
             //string_traits<std::basic_string_view<const volatile CharT> >::size(str) == 4 &&

               std::is_same<string_traits<std::basic_string_view<               CharT> >::char_type, CharT>::value                &&
               std::is_same<string_traits<std::basic_string_view<const          CharT> >::char_type, const          CharT>::value &&
               std::is_same<string_traits<std::basic_string_view<      volatile CharT> >::char_type,       volatile CharT>::value &&
               std::is_same<string_traits<std::basic_string_view<const volatile CharT> >::char_type, const volatile CharT>::value;
    }

} // anonymous namespace


TEST(Encoding, DefaultDeductionPolicy)
{
    EXPECT_TRUE((std::is_same<typename default_deduction_policy::encoding_type<char>, narrow_encoding>::value));
    EXPECT_TRUE((std::is_same<typename default_deduction_policy::encoding_type<wchar_t>, wide_encoding>::value));
}

TEST(Encoding, IsEncodedCharacter)
{
    // default deduction policy
    EXPECT_TRUE(test_is_encoded_character<char>());
    EXPECT_TRUE(test_is_encoded_character<wchar_t>());
    EXPECT_FALSE(test_is_encoded_character<int>());

    // invalid deduction policy
    EXPECT_FALSE((test_is_encoded_character<char,    invalid_deduction_policy>()));
    EXPECT_FALSE((test_is_encoded_character<wchar_t, invalid_deduction_policy>()));
    EXPECT_FALSE((test_is_encoded_character<int,     invalid_deduction_policy>()));
}

TEST(Encoding, IsEncodedString)
{
    // character pointer
    {
        // default deduction policy
        EXPECT_TRUE(test_is_encoded_string_char_ptr<char>());
        EXPECT_TRUE(test_is_encoded_string_char_ptr<wchar_t>());
        EXPECT_FALSE(test_is_encoded_string_char_ptr<int>());

        // invalid deduction policy
        EXPECT_FALSE((test_is_encoded_string_char_ptr<char,    invalid_deduction_policy>()));
        EXPECT_FALSE((test_is_encoded_string_char_ptr<wchar_t, invalid_deduction_policy>()));
        EXPECT_FALSE((test_is_encoded_string_char_ptr<int,     invalid_deduction_policy>()));
    }
    
    // character array
    {
        // default deduction policy
        EXPECT_TRUE(test_is_encoded_string_char_arr<char>());
        EXPECT_TRUE(test_is_encoded_string_char_arr<wchar_t>());
        EXPECT_FALSE(test_is_encoded_string_char_arr<int>());

        // invalid deduction policy
        EXPECT_FALSE((test_is_encoded_string_char_arr<char,    invalid_deduction_policy>()));
        EXPECT_FALSE((test_is_encoded_string_char_arr<wchar_t, invalid_deduction_policy>()));
        EXPECT_FALSE((test_is_encoded_string_char_arr<int,     invalid_deduction_policy>()));
    }
    
    // std::basic_string
    {
        // default deduction policy
        EXPECT_TRUE(test_is_encoded_string_std_basic_string<char>());
        EXPECT_TRUE(test_is_encoded_string_std_basic_string<wchar_t>());
        EXPECT_FALSE(test_is_encoded_string_std_basic_string<int>());

        // invalid deduction policy
        EXPECT_FALSE((test_is_encoded_string_std_basic_string<char,    invalid_deduction_policy>()));
        EXPECT_FALSE((test_is_encoded_string_std_basic_string<wchar_t, invalid_deduction_policy>()));
        EXPECT_FALSE((test_is_encoded_string_std_basic_string<int,     invalid_deduction_policy>()));
    }
    
    // std::basic_string_view
    {
        // default deduction policy
        EXPECT_TRUE(test_is_encoded_string_std_basic_string_view<char>());
        EXPECT_TRUE(test_is_encoded_string_std_basic_string_view<wchar_t>());
        EXPECT_FALSE(test_is_encoded_string_std_basic_string_view<int>());

        // invalid deduction policy
        EXPECT_FALSE((test_is_encoded_string_std_basic_string_view<char,    invalid_deduction_policy>()));
        EXPECT_FALSE((test_is_encoded_string_std_basic_string_view<wchar_t, invalid_deduction_policy>()));
        EXPECT_FALSE((test_is_encoded_string_std_basic_string_view<int,     invalid_deduction_policy>()));
    }
}

TEST(Encoding, EncodingType)
{
    // for a character
    EXPECT_TRUE((std::is_same<encoding_type_t<char>, narrow_encoding>::value));
    EXPECT_TRUE((std::is_same<encoding_type_t<wchar_t>, wide_encoding>::value));

    // for a string
    EXPECT_TRUE((std::is_same<encoding_type_t<const char*>, narrow_encoding>::value));
    EXPECT_TRUE((std::is_same<encoding_type_t<const wchar_t*>, wide_encoding>::value));

    // TODO: with invalid DP ...

    // with an user-provided deduction policy
    //EXPECT_TRUE((std::is_same<deduce_t<char, my_valid_deduction_policy>, narrow_encoding>::value));
}

TEST(Encoding, EncodingTypeIs)
{
    // for a character
    EXPECT_TRUE((encoding_type_is<char, narrow_encoding>::value));
    EXPECT_TRUE((encoding_type_is<wchar_t, wide_encoding>::value));

    // for a string
    EXPECT_TRUE((encoding_type_is<const char*, narrow_encoding>::value));
    EXPECT_TRUE((encoding_type_is<const wchar_t*, wide_encoding>::value));

    // not an encoded string or character
    EXPECT_FALSE((encoding_type_is<int, narrow_encoding>::value));
    EXPECT_FALSE((encoding_type_is<char, narrow_encoding, invalid_deduction_policy>::value));
}

TEST(Encoding, StringTraits)
{
    //string_traits<const std::string>::char_type;

    // character pointer specialization
    {
        EXPECT_TRUE(test_string_traits_char_ptr<char>());
        EXPECT_TRUE(test_string_traits_char_ptr<wchar_t>());
    }

    // character array specialization
    {
        EXPECT_TRUE(test_string_traits_char_arr<char>());
        EXPECT_TRUE(test_string_traits_char_arr<wchar_t>());
    }
    
    // std::basic_string specialization
    {
        EXPECT_TRUE(test_string_traits_std_basic_string<char>());
        EXPECT_TRUE(test_string_traits_std_basic_string<wchar_t>());
    }

    // std::basic_string_view specialization
    {
        EXPECT_TRUE(test_string_traits_std_basic_string_view<char>());
        EXPECT_TRUE(test_string_traits_std_basic_string_view<wchar_t>());
    }
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