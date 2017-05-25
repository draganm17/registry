#pragma once

#include <locale>
#include <memory>
#include <type_traits>

#include <registry/details/string_traits.h>


namespace registry {
namespace details {

    //------------------------------------------------------------------------------------//
    //                                  INTERFACE                                         //
    //------------------------------------------------------------------------------------//

    // Platform-specific narrow characters encoding. ASCII on Windows.
    struct system_narrow_encoding
    {
        using char_type = char;
    };

    // Platform-specific wide characters encoding. UTF-16 on Windows.
    struct system_wide_encoding
    {
        using char_type = wchar_t;
    };

    //struct utf8_encoding
    //{
    //    using char_type = char;
    //};

    //struct utf16_encoding
    //{
    //    using char_type = char16_t;
    //};

    //struct utf32_encoding
    //{
    //    using char_type = char32_t;
    //};

    // Platform specific native encoding type.
    // Windows encoding is UTF-16, encoded in wchar_t characters.
    using native_encoding_type = system_wide_encoding;

    // A trait that queries the default (presumed) encoding type for a given string or characted type.
    // The default encoding type depends on the type of the character deduced from 'T' and is platform-specific.
    // For Windows character types are mapped to encoding types as follows:
    // - 'char'    : 'system_narrow_encoding';
    // - 'wchar_t' : 'system_wide_encoding';
    // The character type 'C' is deduced from 'T' as follows:
    // - if 'is_character<T>::value == true', then 'C' is deduced as 'std::remove_cv_t<T>';
    // - if 'is_string<T>::value == true', then 'C' is deduced as 'string_traits<T>::char_type'.
    // Otherwise the expression is ill-formed.
    template <typename T, typename = void> struct default_encoding;
 // {
        // using type = /* default encoding type for 'T' */
 // };

    // A traits that queries the default codec for 'T'.
    // The default codec type is deduced as 'string_codec<default_encoding<T>::type>'.
    template <typename T> struct default_codec;
 // {
        // using type = /* default codec for 'T' */
 // };

    // A traits that queries if 'T' has an default encoding type,
    // i.e. if the expression 'default_encoding<T>::type' is valid.
    template <typename T, typename = void> struct has_default_encoding;

    // The base class for all string codecs.
    template <typename Encoding,
              typename EncAlloc = std::allocator<typename Encoding::char_type>,
              typename DecAlloc = std::allocator<typename native_encoding_type::char_type>
    >
    class string_codec_base
    {
    public:
        // The type of the encoded string.
        using encoded_string_type = std::basic_string<typename Encoding::char_type,
                                                      std::char_traits<typename Encoding::char_type>, EncAlloc>;

        // The type of the decoded string.
        using decoded_string_type = std::basic_string<typename native_encoding_type::char_type,
                                                      std::char_traits<typename native_encoding_type::char_type>, DecAlloc>;

    protected:
        string_codec_base() = default;

        string_codec_base(const string_codec_base&) = delete;

        string_codec_base& operator=(const string_codec_base&) = delete;

        ~string_codec_base() = default;
    };

    // String codec.
    // Encodes strings from the 'native_encoding_type' to 'Encoding' and back.
    // This class is specialized for each supported encoding types.
    template <typename Encoding,
              typename EncAlloc = std::allocator<typename Encoding::char_type>,
              typename DecAlloc = std::allocator<typename native_encoding_type::char_type>
    >
    class string_codec /* : public string_codec_base<Encoding, EncAlloc, DecAlloc> */;
 // {
      // encoded_string_type encode(const typename native_encoding_type::char_type* first,
      //                            const typename native_encoding_type::char_type* last,
      //                            const std::locale& loc = std::locale(), const EncAlloc& alloc = EncAlloc())
      // {
      //     /* Encodes a string in 'native_encoding_type' encoding to the 'Encoding' encoding. */
      // }

      // decoded_string_type decode(const typename Encoding::char_type* first,
      //                            const typename Encoding::char_type* last,
      //                            const std::locale& loc = std::locale(), const EncAlloc& alloc = EncAlloc());
      // {
      //     /* Decodes a string from 'Encoding' encoding to the 'native_encoding_type' encoding. */
      // }
 // };


    //------------------------------------------------------------------------------------//
    //                            IMPLEMENTATION DETAILS                                  //
    //------------------------------------------------------------------------------------//

    template< class... >
    using void_t = void;

    template <typename T, typename = void> struct default_encoding { };

    template <typename T>
    struct default_encoding<T, std::enable_if_t<is_character<T>::value>>
    {
    private:
        static auto get(char)     -> system_narrow_encoding;
        static auto get(wchar_t)  -> system_wide_encoding;
     // static auto get(char16_t) -> utf16_encoding;
     // static auto get(char32_t) -> utf32_encoding;

    public:
        using type = decltype(get(std::remove_cv_t<T>{}));
    };

    template <typename T>
    struct default_encoding<T, std::enable_if_t<is_string<T>::value>>
    {
        using type = typename default_encoding<typename string_traits<T>::char_type>::type;
    };

    template <typename T> struct default_codec
    {
        using type = string_codec<typename default_encoding<T>::type>;
    };

    template <typename T, typename = void>
    struct has_default_encoding : std::false_type { };

    template <typename T>
    struct has_default_encoding<T, void_t<typename default_encoding<T>::type>> : std::true_type { };

    template<typename CharT, typename OutStr, typename Codecvt, typename State, typename Fn>
    bool do_str_codecvt(const CharT* first, const CharT* last, OutStr& out,
                        const Codecvt& cvt, State& state, size_t& count, Fn fn)
    {
        if (first == last) {
            count = 0;
            out.clear();
	        return true;
	    }

        auto next = first;
        size_t outchars = 0;
        std::codecvt_base::result result;
        const auto maxlen = cvt.max_length() + 1;

        do {
            out.resize(out.size() + (last - next) * maxlen);
            auto outnext = &out.front() + outchars;
            auto const outlast = &out.back() + 1;
            result = (cvt.*fn)(state, next, last, next, outnext, outlast, outnext);
            outchars = outnext - &out.front();
	    } while (result == std::codecvt_base::partial && next != last && (out.size() - outchars) < maxlen);

        if (result == std::codecvt_base::error) {
	        count = next - first;
	        return false;
	    } else if (result == std::codecvt_base::noconv) {
            out.assign(first, last);
	        count = last - first;
	    } else {
            out.resize(outchars);
	        count = next - first;
	    }
    
        return true;
    }

    template<typename CharT, typename Traits, typename Alloc, typename State>
    inline bool str_codecvt_in(const char* first, const char* last,
                               std::basic_string<CharT, Traits, Alloc>& out,
                               const std::codecvt<CharT, char, State>& cvt)
    {
        size_t count;
        State state{};
        return do_str_codecvt(first, last, out, cvt, state, count, &std::codecvt<CharT, char, State>::in);
    }

    template<typename CharT, typename Traits, typename Alloc, typename State>
    inline bool str_codecvt_out(const CharT* first, const CharT* last,
		                        std::basic_string<char, Traits, Alloc>& out,
		                        const std::codecvt<CharT, char, State>& cvt)
    {
        size_t count;
        State state{};
        return do_str_codecvt(first, last, out, cvt, state, count, &std::codecvt<CharT, char, State>::out);
    }

    // String codec specialization for 'system_narrow_encoding'.
    template <typename EncAlloc, typename DecAlloc>
    class string_codec<system_narrow_encoding, EncAlloc, DecAlloc>
    : public string_codec_base<system_narrow_encoding, EncAlloc, DecAlloc>
    {
    public:
        encoded_string_type encode(const wchar_t* first, const wchar_t* last,
                                   const std::locale& loc = std::locale(), const EncAlloc& alloc = EncAlloc())
        {
            encoded_string_type out(alloc);
            using Codecvt = std::codecvt<wchar_t, char, std::mbstate_t>;
            if (!str_codecvt_out(first, last, out, std::use_facet<Codecvt>(loc))) {
                throw std::range_error("Bad conversion");
            }
            return out;
        }

        decoded_string_type decode(const char* first, const char* last,
                                   const std::locale& loc = std::locale(), const DecAlloc& alloc = DecAlloc())
        {
            decoded_string_type out(alloc);
            using Codecvt = std::codecvt<wchar_t, char, std::mbstate_t>;
            if (!str_codecvt_in(first, last, out, std::use_facet<Codecvt>(loc))) {
                throw std::range_error("Bad conversion");
            }
            return out;
        }
    };

    // String codec specialization for 'system_wide_encoding'.
    template <typename EncAlloc, typename DecAlloc>
    class string_codec<system_wide_encoding, EncAlloc, DecAlloc>
    : public string_codec_base<system_wide_encoding, EncAlloc, DecAlloc>
    {
    public:
        encoded_string_type encode(const wchar_t* first, const wchar_t* last,
                                   const std::locale& loc = std::locale(), const EncAlloc& alloc = EncAlloc())
        {
            encoded_string_type out(first, last - first, alloc);
            return out;
        }

        decoded_string_type decode(const wchar_t* first, const wchar_t* last,
                                   const std::locale& loc = std::locale(), const DecAlloc& alloc = DecAlloc())
        {
            decoded_string_type out(first, last - first, alloc);
            return out;
        }
    };

}} // namespace registry::details