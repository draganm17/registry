#include <string>
#include <string_view>
#include <type_traits>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/details/string_traits.h>

using namespace registry::details::encoding;

namespace {

    template <typename CharT>
    inline bool test_is_character()
    {
        return is_character<               CharT>::value &&
               is_character<const          CharT>::value &&
               is_character<      volatile CharT>::value &&
               is_character<const volatile CharT>::value;
    }

    template <typename CharT>
    inline bool test_is_string_char_ptr()
    {
        return is_string< CharT                *                >::value &&
               is_string< CharT const          *                >::value &&
               is_string< CharT                * const          >::value &&
               is_string< CharT const          * const          >::value &&
               is_string< CharT                *                >::value &&
               is_string< CharT       volatile *                >::value &&
               is_string< CharT                *       volatile >::value &&
               is_string< CharT       volatile *       volatile >::value &&
               is_string< CharT                *                >::value &&
               is_string< CharT const volatile *                >::value &&
               is_string< CharT                * const volatile >::value &&
               is_string< CharT const volatile * const volatile >::value;
    }

    template <typename CharT>
    inline bool test_is_string_char_arr()
    {
        return is_string<                CharT[5] >::value &&
               is_string< const          CharT[5] >::value &&
               is_string<       volatile CharT[5] >::value &&
               is_string< const volatile CharT[5] >::value;
    }

    template <typename CharT>
    inline bool test_is_string_std_basic_string()
    {
        return is_string<                std::basic_string<CharT>>::value &&
               is_string< const          std::basic_string<CharT>>::value &&
               is_string<       volatile std::basic_string<CharT>>::value &&
               is_string< const volatile std::basic_string<CharT>>::value;
    }

    template <typename CharT>
    inline bool test_is_string_std_basic_string_view()
    {
        return is_string<                std::basic_string_view<               CharT>>::value &&
               is_string< const          std::basic_string_view<               CharT>>::value &&
               is_string<       volatile std::basic_string_view<               CharT>>::value &&
               is_string< const volatile std::basic_string_view<               CharT>>::value &&
               is_string<                std::basic_string_view<const          CharT>>::value &&
               is_string<                std::basic_string_view<      volatile CharT>>::value &&
               is_string<                std::basic_string_view<const volatile CharT>>::value;
    }

    template <typename CharT>
    bool test_string_traits_char_ptr()
    {
        CharT str[] = { CharT('T'), CharT('E'), CharT('S'), CharT('T'), CharT('\0') };

        return string_traits< CharT                *                >::data(str) == &str[0] &&
               string_traits< CharT const          *                >::data(str) == &str[0] &&
               string_traits< CharT                * const          >::data(str) == &str[0] &&
               string_traits< CharT const          * const          >::data(str) == &str[0] &&
               string_traits< CharT                *                >::data(str) == &str[0] &&
             //string_traits< CharT       volatile *                >::data(str) == &str[0] &&
             //string_traits< CharT                *       volatile >::data(str) == &str[0] &&
             //string_traits< CharT       volatile *       volatile >::data(str) == &str[0] &&
               string_traits< CharT                *                >::data(str) == &str[0] &&
             //string_traits< CharT const volatile *                >::data(str) == &str[0] &&
             //string_traits< CharT                * const volatile >::data(str) == &str[0] &&
             //string_traits< CharT const volatile * const volatile >::data(str) == &str[0] &&

               string_traits< CharT                *                >::size(str) == 4 &&
               string_traits< CharT const          *                >::size(str) == 4 &&
               string_traits< CharT                * const          >::size(str) == 4 &&
               string_traits< CharT const          * const          >::size(str) == 4 &&
               string_traits< CharT                *                >::size(str) == 4 &&
             //string_traits< CharT       volatile *                >::size(str) == 4 &&
             //string_traits< CharT                *       volatile >::size(str) == 4 &&
             //string_traits< CharT       volatile *       volatile >::size(str) == 4 &&
               string_traits< CharT                *                >::size(str) == 4 &&
             //string_traits< CharT const volatile *                >::size(str) == 4 &&
             //string_traits< CharT                * const volatile >::size(str) == 4 &&
             //string_traits< CharT const volatile * const volatile >::size(str) == 4 &&

               std::is_same<string_traits< CharT                *                >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT const          *                >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT                * const          >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT const          * const          >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT                *                >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT       volatile *                >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT                *       volatile >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT       volatile *       volatile >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT                *                >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT const volatile *                >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT                * const volatile >::char_type, CharT>::value &&
               std::is_same<string_traits< CharT const volatile * const volatile >::char_type, CharT>::value;
    }

    template <typename CharT>
    bool test_string_traits_char_arr()
    {
        CharT str[] = { CharT('T'), CharT('E'), CharT('S'), CharT('T'), CharT('\0') };

        return string_traits<                CharT[5] >::data(str) == &str[0] &&
               string_traits< const          CharT[5] >::data(str) == &str[0] &&
             //string_traits<       volatile CharT[5] >::data(str) == &str[0] &&
             //string_traits< const volatile CharT[5] >::data(str) == &str[0] &&

               string_traits<                CharT[5] >::size(str) == 4 &&
               string_traits< const          CharT[5] >::size(str) == 4 &&
             //string_traits<       volatile CharT[5] >::size(str) == 4 &&
             //string_traits< const volatile CharT[5] >::size(str) == 4 &&

               std::is_same<string_traits<                CharT[5] >::char_type, CharT>::value &&
               std::is_same<string_traits< const          CharT[5] >::char_type, CharT>::value &&
               std::is_same<string_traits<       volatile CharT[5] >::char_type, CharT>::value &&
               std::is_same<string_traits< const volatile CharT[5] >::char_type, CharT>::value;
    }

    template <typename CharT>
    bool test_string_traits_std_basic_string()
    {
        std::basic_string<CharT> str(4, CharT('x'));

        return string_traits<                std::basic_string<CharT> >::data(str) == &str[0] &&
               string_traits< const          std::basic_string<CharT> >::data(str) == &str[0] &&
             //string_traits<       volatile std::basic_string<CharT> >::data(str) == &str[0] &&
             //string_traits< const volatile std::basic_string<CharT> >::data(str) == &str[0] &&

               string_traits<                std::basic_string<CharT> >::size(str) == 4 &&
               string_traits< const          std::basic_string<CharT> >::size(str) == 4 &&
             //string_traits<       volatile std::basic_string<CharT> >::size(str) == 4 &&
             //string_traits< const volatile std::basic_string<CharT> >::size(str) == 4 &&

               std::is_same<string_traits<                std::basic_string<CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits< const          std::basic_string<CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits<       volatile std::basic_string<CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits< const volatile std::basic_string<CharT> >::char_type, CharT>::value;
    }

    template <typename CharT>
    bool test_string_traits_std_basic_string_view()
    {
        std::basic_string<CharT> s(4, CharT('x'));
        std::basic_string_view<CharT> str = s;

        return string_traits<                std::basic_string_view<               CharT> >::data(str) == &str[0] &&
               string_traits< const          std::basic_string_view<               CharT> >::data(str) == &str[0] &&
             //string_traits<       volatile std::basic_string_view<               CharT> >::data(str) == &str[0] &&
             //string_traits< const volatile std::basic_string_view<               CharT> >::data(str) == &str[0] &&
             //string_traits<                std::basic_string_view<const          CharT> >::data(str) == &str[0] &&
             //string_traits<                std::basic_string_view<      volatile CharT> >::data(str) == &str[0] &&
             //string_traits<                std::basic_string_view<const volatile CharT> >::data(str) == &str[0] &&

               string_traits<                std::basic_string_view<               CharT> >::size(str) == 4 &&
               string_traits< const          std::basic_string_view<               CharT> >::size(str) == 4 &&
             //string_traits<       volatile std::basic_string_view<               CharT> >::size(str) == 4 &&
             //string_traits< const volatile std::basic_string_view<               CharT> >::size(str) == 4 &&
             //string_traits<                std::basic_string_view<const          CharT> >::size(str) == 4 &&
             //string_traits<                std::basic_string_view<      volatile CharT> >::size(str) == 4 &&
             //string_traits<                std::basic_string_view<const volatile CharT> >::size(str) == 4 &&

               std::is_same<string_traits<                std::basic_string_view<               CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits< const          std::basic_string_view<               CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits<       volatile std::basic_string_view<               CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits< const volatile std::basic_string_view<               CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits<                std::basic_string_view<const          CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits<                std::basic_string_view<      volatile CharT> >::char_type, CharT>::value &&
               std::is_same<string_traits<                std::basic_string_view<const volatile CharT> >::char_type, CharT>::value;
    }

} // anonymous namespace


TEST(Encoding, IsCharacter)
{
    EXPECT_TRUE(test_is_character<char>());
    EXPECT_TRUE(test_is_character<signed char>());
    EXPECT_TRUE(test_is_character<unsigned char>());
    EXPECT_TRUE(test_is_character<wchar_t>());
    EXPECT_TRUE(test_is_character<char16_t>());
    EXPECT_TRUE(test_is_character<char32_t>());
    EXPECT_FALSE(test_is_character<int>());
}

TEST(Encoding, IsString)
{
    // character pointer
    {
        EXPECT_TRUE(test_is_string_char_ptr<char>());
        EXPECT_TRUE(test_is_string_char_ptr<signed char>());
        EXPECT_TRUE(test_is_string_char_ptr<unsigned char>());
        EXPECT_TRUE(test_is_string_char_ptr<wchar_t>());
        EXPECT_TRUE(test_is_string_char_ptr<char16_t>());
        EXPECT_TRUE(test_is_string_char_ptr<char32_t>());
        EXPECT_FALSE(test_is_string_char_ptr<int>());
    }

    // character array
    {
        EXPECT_TRUE(test_is_string_char_arr<char>());
        EXPECT_TRUE(test_is_string_char_arr<signed char>());
        EXPECT_TRUE(test_is_string_char_arr<unsigned char>());
        EXPECT_TRUE(test_is_string_char_arr<wchar_t>());
        EXPECT_TRUE(test_is_string_char_arr<char16_t>());
        EXPECT_TRUE(test_is_string_char_arr<char32_t>());
        EXPECT_FALSE(test_is_string_char_arr<int>());
    }

    // std::basic_string
    {
        EXPECT_TRUE(test_is_string_std_basic_string<char>());
        EXPECT_TRUE(test_is_string_std_basic_string<signed char>());
        EXPECT_TRUE(test_is_string_std_basic_string<unsigned char>());
        EXPECT_TRUE(test_is_string_std_basic_string<wchar_t>());
        EXPECT_TRUE(test_is_string_std_basic_string<char16_t>());
        EXPECT_TRUE(test_is_string_std_basic_string<char32_t>());
        EXPECT_FALSE(test_is_string_std_basic_string<int>());
    }

    // std::basic_string_view
    {
        EXPECT_TRUE(test_is_string_std_basic_string_view<char>());
        EXPECT_TRUE(test_is_string_std_basic_string_view<signed char>());
        EXPECT_TRUE(test_is_string_std_basic_string_view<unsigned char>());
        EXPECT_TRUE(test_is_string_std_basic_string_view<wchar_t>());
        EXPECT_TRUE(test_is_string_std_basic_string_view<char16_t>());
        EXPECT_TRUE(test_is_string_std_basic_string_view<char32_t>());
        EXPECT_FALSE(test_is_string_std_basic_string_view<int>());
    }
}

TEST(Encoding, StringTraits)
{
    // character pointer specialization
    {
        EXPECT_TRUE(test_string_traits_char_ptr<char>());
        EXPECT_TRUE(test_string_traits_char_ptr<signed char>());
        EXPECT_TRUE(test_string_traits_char_ptr<unsigned char>());
        EXPECT_TRUE(test_string_traits_char_ptr<wchar_t>());
        EXPECT_TRUE(test_string_traits_char_ptr<char16_t>());
        EXPECT_TRUE(test_string_traits_char_ptr<char32_t>());
        //EXPECT_FALSE(test_string_traits_char_ptr<int>());
    }

    // character array specialization
    {
        EXPECT_TRUE(test_string_traits_char_arr<char>());
        EXPECT_TRUE(test_string_traits_char_arr<signed char>());
        EXPECT_TRUE(test_string_traits_char_arr<unsigned char>());
        EXPECT_TRUE(test_string_traits_char_arr<wchar_t>());
        EXPECT_TRUE(test_string_traits_char_arr<char16_t>());
        EXPECT_TRUE(test_string_traits_char_arr<char32_t>());
        //EXPECT_FALSE(test_string_traits_char_arr<int>());
    }

    // std::basic_string specialization
    {
        EXPECT_TRUE(test_string_traits_std_basic_string<char>());
        EXPECT_TRUE(test_string_traits_std_basic_string<signed char>());
        EXPECT_TRUE(test_string_traits_std_basic_string<unsigned char>());
        EXPECT_TRUE(test_string_traits_std_basic_string<wchar_t>());
        EXPECT_TRUE(test_string_traits_std_basic_string<char16_t>());
        EXPECT_TRUE(test_string_traits_std_basic_string<char32_t>());
        //EXPECT_FALSE(test_string_traits_std_basic_string<int>());
    }

    // std::basic_string_view specialization
    {
        EXPECT_TRUE(test_string_traits_std_basic_string_view<char>());
        EXPECT_TRUE(test_string_traits_std_basic_string_view<signed char>());
        EXPECT_TRUE(test_string_traits_std_basic_string_view<unsigned char>());
        EXPECT_TRUE(test_string_traits_std_basic_string_view<wchar_t>());
        EXPECT_TRUE(test_string_traits_std_basic_string_view<char16_t>());
        EXPECT_TRUE(test_string_traits_std_basic_string_view<char32_t>());
        //EXPECT_FALSE(test_string_traits_std_basic_string_view<int>());
    }
}