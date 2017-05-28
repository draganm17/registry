#pragma once

#include <locale>
#include <memory>
#include <type_traits>

#include <registry/details/string_traits.h>


namespace registry {
namespace details {
namespace encoding {

    //------------------------------------------------------------------------------------//
    //                                  INTERFACE                                         //
    //------------------------------------------------------------------------------------//

    // Platform-specific narrow characters encoding.
    // ASCII on Windows.
    struct narrow_encoding
    {
        using char_type = char;
    };

    // Platform-specific wide characters encoding.
    // UTF-16 on Windows.
    struct wide_encoding
    {
        using char_type = wchar_t;
    };

    // Platform specific native encoding type.
    // Windows encoding is UTF-16, encoded in wchar_t characters.
    using native_encoding_type = wide_encoding;

    // Checks whether 'T' is an encoding type.
    // Can be used as a customization point.
    // Enabled specializations should derrive from 'std::true_type'.
    // Specializations are already enabled for 'narrow_encoding' and 'wide_encoding'.
    // NOTE: TODO: ...
    template <typename T, typename Enable = void>
    struct is_encoding : std::false_type { };

    // TODO: ...
    struct default_deduction_policy;
 // {
        // template<typename CharT>
        // using encoding_type = /* dedcued encoding type for 'CharT' */
 // };

    // Checks whether an encoding type can be deduced for 'T', which
    // is a string or a character type, using the 'DP' deduction policy.
    // Derrives from 'std::true_type' if all of the following conditions are true:
    // - 'T' is a string or a character type,
    //   i.e. 'is_string<T>::value || is_character<T>::value' is true;
    // - 'DP' is an deduction policy that produces an valid encoding type,
    //   i.e. 'is_encoding<typename DP::template encoding_type<character_type_t<T>>>::value' is true.
    // Otherwise, derrives from 'std::false type'.
    template <typename T,
              typename DP = default_deduction_policy,
              typename = void
    >
    struct is_deducible : std::false_type { };

    // Checks whether an encoding type deduced for 'T' (if any) using
    // the 'DP' deduction policy is the same as 'E', which in encoding type.
    // Derrives from 'std::true_type' if all of the following conditions are true:
    // - 'DP' is able to deduce an encoding type fro 'T', i.e. 'is_deducible<T, DP>::value' is true;
    // - 'E' is an encoding type, i.e. 'is_encoding<E>::value' is true;
    // - The encoding type deduced by 'DP' is equal to 'E', 
    //   i.e. 'std::is_same<typename DP::template encoding_type<character_type_t<T>>, E>::value' is true.
    // Otherwise, derrives from 'std::false type'.
    template <typename T,
              typename E,
              typename DP = default_deduction_policy,
              typename = void
    >
    struct is_deducible_to : std::false_type { };

    // Deduces the encoding type for 'T', which is a string or characted type.
    // The encoding type depends on the type of the character deduced from 'T' and is platform-specific.
    // For Windows character types are mapped to encoding types as follows:
    // - 'char'    : 'narrow_encoding';
    // - 'wchar_t' : 'wide_encoding';
    // The character type 'C' is deduced from 'T' as follows:
    // - if 'is_character<T>::value == true', then 'C' is deduced as 'std::remove_cv_t<T>';
    // - if 'is_string<T>::value == true', then 'C' is deduced as 'string_traits<T>::char_type'.
    // Otherwise the expression is ill-formed.
    
    // Deduces the encoding type for 'T', which is a string or characted type.
    // - ...
    // TODO: ...
    template <typename T,
              typename DP = default_deduction_policy
    >
    struct deduce
    {
        using type = typename DP::template encoding_type<character_type_t<T>>;
    };

    // Helper type
    template<typename T, typename DP = default_deduction_policy>
    using deduce_t = typename deduce<T>::type;

    // The base class for all enabled specializations of 'codec'.
    template <typename Encoding,
              typename EncAlloc = std::allocator<typename Encoding::char_type>,
              typename DecAlloc = std::allocator<typename native_encoding_type::char_type>
    >
    class codec_base
    {
    public:
        using encoding_type = Encoding;

        using encoding_allocator_type = EncAlloc;

        using decoding_allocator_type = DecAlloc;

        // The type of the encoded string.
        using encoded_string_type = std::basic_string<typename Encoding::char_type,
                                                      std::char_traits<typename Encoding::char_type>, EncAlloc>;

        // The type of the decoded string.
        using decoded_string_type = std::basic_string<typename native_encoding_type::char_type,
                                                      std::char_traits<typename native_encoding_type::char_type>, DecAlloc>;

    protected:
        codec_base() noexcept = default;

        codec_base(const codec_base&) = delete;

        codec_base& operator=(const codec_base&) = delete;

        ~codec_base() = default;
    };

    // String codec.
    // Converts strings from 'native_encoding_type' to 'Encoding' and back.
    // Enabled specializations should derrive from 'codec_base' and provide members as shown below.
    // NOTE: This class should have an enabled specialization for each
    //       type 'T' for which 'is_encoding<T>' specialization is enabled.
    template <typename Encoding,
              typename EncAlloc = std::allocator<typename Encoding::char_type>,
              typename DecAlloc = std::allocator<typename native_encoding_type::char_type>,
              typename Enabled =  void
    >
    class codec /* : public codec_base<Encoding, EncAlloc, DecAlloc> */
    {
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
    };


    //------------------------------------------------------------------------------------//
    //                            IMPLEMENTATION DETAILS                                  //
    //------------------------------------------------------------------------------------//

    template< class... >
    using void_t = void;

    template <typename T>
    struct is_encoding<T, std::enable_if_t<std::is_same<T, narrow_encoding>::value ||
                                           std::is_same<T, wide_encoding>::value>>
    : std::true_type { };

    struct default_deduction_policy
    {
    private:
        template <typename T> struct get { };
        template <> struct get<char>     { using type = narrow_encoding; };
        template <> struct get<wchar_t>  { using type = wide_encoding; };

    public:
        template<typename CharT>
        using encoding_type = typename get<CharT>::type;
    };

    template <typename T, typename DP>
    struct is_deducible<T, DP,
                        std::enable_if_t<(is_string<T>::value || is_character<T>::value) &&
                                          is_encoding<typename DP::template encoding_type<character_type_t<T>>>::value>>
    : std::true_type { };

    template <typename T, typename E, typename DP>
    struct is_deducible_to<T, E, DP,
                           std::enable_if_t<is_deducible<T>::value && is_encoding<E>::value  &&
                                            std::is_same<typename DP::template encoding_type<character_type_t<T>>, E>::value>>
    : std::true_type { };

    template <typename T>
    struct deduce<T, std::enable_if_t<std::is_same<std::remove_cv_t<T>, char>::value ||
                                      std::is_same<std::remove_cv_t<T>, wchar_t>::value>>
    {
    private:
        static auto get(char)     -> narrow_encoding;
        static auto get(wchar_t)  -> wide_encoding;
     // static auto get(char16_t) -> utf16_encoding;
     // static auto get(char32_t) -> utf32_encoding;

    public:
        using type = decltype(get(std::remove_cv_t<T>{}));
    };

    template <typename T>
    struct deduce<T, void_t<std::enable_if_t<is_string<T>::value>,
                            deduce_t<typename string_traits<T>::char_type>>>
    {
        using type = deduce_t<typename string_traits<T>::char_type>;
    };

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

    // String codec specialization for 'narrow_encoding'.
    template <typename EncAlloc, typename DecAlloc>
    class codec<narrow_encoding, EncAlloc, DecAlloc>
    : public codec_base<narrow_encoding, EncAlloc, DecAlloc>
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

    // String codec specialization for 'wide_encoding'.
    template <typename EncAlloc, typename DecAlloc>
    class codec<wide_encoding, EncAlloc, DecAlloc>
    : public codec_base<wide_encoding, EncAlloc, DecAlloc>
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
    
}}} // namespace registry::details::encoding