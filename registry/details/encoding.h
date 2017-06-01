#pragma once

#include <locale>
#include <memory>
#include <string>
#if _HAS_CXX17
#include <string_view>
#endif
#include <type_traits>


namespace registry {
namespace details {
namespace encoding {

    //------------------------------------------------------------------------------------//
    //                                  INTERFACE                                         //
    //------------------------------------------------------------------------------------//

    // NOTE: Encodings ...
    // TODO: ^^^

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
    // Windows encoding type is 'wide_encoding'.
    using native_encoding_type = wide_encoding;

    // The default policy used to deduce the encoding type from a given character type.
    //
    // On Windows the deduction rules are the follow:
    // - 'char'    -> 'narrow_encoding';
    // - 'wchar_t' -> 'wide_encoding';
    //
    // Users can provide their own deduction policies. Each policy
    // should provide a template member typedef 'encoding_type' as shown below.
    //
    // NOTE: The type of the character passed to the deduction policy should correspond to the
    //       type of the character associated with the encoding type the deduction policy produces.
    //       Specifically, let 'DP' be an deduction policy and 'CharT' an character type. Then 
    //       'std::is_same<DP::encoding_type<CharT>::char_type, CharT>::value' should be true.
    //       Otherwise, the behaviour is undefined.
    struct default_deduction_policy;
 // {
        // template<typename CharT>
        // using encoding_type = /* dedcued encoding type for 'CharT' */
 // };

    // Checks whether 'T' is an encoded character type, i.e. a type of
    // character which encoding can be deduced by the 'DP' deduction policy.
    // The const-volatile qualification of 'T' is dropped when passed to 'DP'.
    //
    // Derrives from 'std::true_type' if 'DP::encoding_type<std::remove_cv_t<T>>' is well-formed.
    // Otherwise, derrives from 'std::false type'.
    template <typename T, 
              typename DP = default_deduction_policy
    >
    struct is_encoded_character;

    // Checks whether 'T' is an encoded string type, i.e. a type of string which character
    // type 'C' (a) can be deduced by 'string_traits', and (b) is an encoded character type,
    // i.e. the encoding type for 'C' can be deduced by the 'DP' deduction policy.
    // The const-volatile qualification of 'T' is dropped when passed to 'string_traits'.
    // The const-volatile qualification of 'C' is dropped when passed to 'DP'.
    //
    // Derrives from 'std::true_type' if 'string_traits<std::remove_cv_t<T>>::char_type' is well-formed
    // and 'is_encoded_character<string_traits<std::remove_cv_t<T>>::char_type, DP>::value' is true.
    // Otherwise, derrives from 'std::false type'.
    template <typename T, 
              typename DP = default_deduction_policy
    >
    struct is_encoded_string;

    // Deduces the encoding type for 'T', which is an encoded
    // string or an encoded characted type, using the 'DP' deduction policy.
    //
    // If 'is_encoded_character<T, DP>::value' is true, the member typedef 'type' is 
    // 'DP::encoding_type<std::remove_cv_t<T>>'.
    // Otherwise, if 'is_encoded_string<T, DP>::value' is true, the member typedef 'type' is 
    // 'DP::encoding_type<std::remove_cv_t<string_traits<std::remove_cv_t<T>>::char_type>>'.
    // Otherwise, the expression 'encoded_character_type<T>::type' is ill-formed.
    template <typename T,
              typename DP = default_deduction_policy
    >
    struct encoding_type;

    // Helper type
    template<typename T, 
             typename DP = default_deduction_policy
    >
    using encoding_type_t = typename encoding_type<T>::type;

    // Checks whether the encoding type deduced for 'T', which is an encoded string or
    // an encoded character type, by 'DP', which is an deduction policy, is the same as 'E'.
    //
    // Derrives from 'std::true_type' if 'encoding_type<T, DP>::type' is
    // well-formed and 'std::is_same<encoding_type<T, DP>::type, E>::value' is true.
    // Otherwise, derrives from 'std::false type'.
    template <typename T,
              typename E,
              typename DP = default_deduction_policy
    >
    struct encoding_type_is;

    // Traits for strings.
    //
    // The default, non - specialized, 'string_traits' accepts the following types:
    // - Encoded character pointers.
    //   i.e. 'T' is a pointer to 'C', and 'is_encoded_character<C>::value' is true;
    // - Encoded character arrays.
    //   i.e. 'T' is an array of 'C', and 'is_encoded_character<C>::value' is true;
    // - Standart strings, which character types are encoded characters.
    //   i.e. 'T' is an 'std::basic_string<C, T, A>', and 'is_encoded_character<C>::value' is true;
    // - Standart string views, which character types are encoded characters. (Only if C++17 is supported).
    //   i.e. 'T' is an 'std::basic_string_view<C, T>', and 'is_encoded_character<C>::value' is true.
    //
    // Users can specialize this class if they wish to support custom string types.
    // Specializations should provide members as shown below.
    //
    // TODO: what is the string traits and deduction polisy relation ???
    template <typename T>
    struct string_traits;
 // {
        // using char_type = /* (possibly cv-qualified) character type */

        // static size_t size(const T&)           { /* returns the string length */ }
        // static const char_type* data(const T&) { /* returns the string data */ }
 // };

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
    //
    // The following specializations are already provided by the library:
    // - 'codec<narrow_encoding>';
    // - 'codec<wide_encoding>'.
    //
    // Specializations should derrive from 'codec_base' and provide members as shown below.
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

    template <typename...>
    using void_t = void;

    struct default_deduction_policy
    {
    private:
        template <typename T> struct get { };
        template <> struct get<char>     { using type = narrow_encoding; };
        template <> struct get<wchar_t>  { using type = wide_encoding; };

    public:
        template <typename CharT>
        using encoding_type = typename get<CharT>::type;
    };

    template <typename T, typename DP, typename = void>
    struct is_encoded_character_impl : std::false_type { };

    template <typename T, typename DP>
    struct is_encoded_character_impl<T, DP, void_t<typename DP::template encoding_type<T>>>
    : std::true_type { };

    template <typename T, typename DP = default_deduction_policy>
    struct is_encoded_character : is_encoded_character_impl<std::remove_cv_t<T>, DP> { };

    template <typename T, typename DP, typename = void>
    struct is_encoded_string_impl : std::false_type { };

    template <typename T, typename DP>
    struct is_encoded_string_impl<T, DP, 
                                  std::enable_if_t<is_encoded_character<typename string_traits<T>::char_type, DP>::value>>
    : std::true_type { };

    template <typename T, typename DP = default_deduction_policy>
    struct is_encoded_string : is_encoded_string_impl<std::remove_cv_t<T>, DP> { };

    template <typename T, typename DP, typename = void>
    struct encoding_type_impl { };

    template <typename T, typename DP>
    struct encoding_type_impl<T, DP, std::enable_if_t<is_encoded_character<T, DP>::value>>
    {
        using type = typename DP::template encoding_type<T>;
    };

    template <typename T, typename DP>
    struct encoding_type_impl<T, DP, std::enable_if_t<is_encoded_string<T, DP>::value>>
    {
        using type = typename DP::template encoding_type<std::remove_cv_t<typename string_traits<T>::char_type>>;
    };

    template <typename T, typename DP = default_deduction_policy>
    struct encoding_type : encoding_type_impl<std::remove_cv_t<T>, DP> { };

    template <typename T, typename E, typename DP, typename = void>
    struct encoding_type_is_impl : std::false_type { };

    template <typename T, typename E, typename DP>
    struct encoding_type_is_impl<T, E, DP, std::enable_if_t<std::is_same<typename encoding_type<T, DP>::type, E>::value>>
    : std::true_type { };

    template <typename T, typename E, typename DP = default_deduction_policy>
    struct encoding_type_is : encoding_type_is_impl<T, E, DP> { };

    template <typename T, typename = void>
    struct is_encoded_character_pointer : std::false_type { };

    template <typename T>
    struct is_encoded_character_pointer<T*, typename std::enable_if_t<is_encoded_character<T>::value>>
    : std::true_type { };

    template <typename T, typename = void>
    struct is_encoded_character_array : std::false_type { };

    template <typename T, size_t N>
    struct is_encoded_character_array<T[N], typename std::enable_if_t<is_encoded_character<T>::value>> 
    : std::true_type { };

    template <typename T, typename = void>
    struct is_std_encoded_character_string : std::false_type { };

    template <typename CharT, typename Traits, typename Alloc>
    struct is_std_encoded_character_string<std::basic_string<CharT, Traits, Alloc>,
                                           typename std::enable_if_t<is_encoded_character<CharT>::value>>
    : std::true_type { };

#if _HAS_CXX17
    template <typename T, typename = void>
    struct is_std_encoded_character_string_view : std::false_type { };

    template <typename CharT, typename Traits>
    struct is_std_encoded_character_string_view<std::basic_string_view<CharT, Traits>,
                                                typename std::enable_if_t<is_encoded_character<CharT>::value>>
    : std::true_type { };
#endif

    template <typename T, typename = void>
    struct string_traits_impl { };

    template <typename T>
    struct string_traits_impl<T, typename std::enable_if_t<is_encoded_character_pointer<T>::value>>
    {
        using char_type = std::remove_pointer_t<T>;

        static size_t size(const T& str) noexcept           { return std::char_traits<char_type>::length(str); }
        static const char_type* data(const T& str) noexcept { return str; }
    };

    template <typename T>
    struct string_traits_impl<T, typename std::enable_if_t<is_encoded_character_array<T>::value>>
    {
        using char_type = std::remove_extent_t<T>;

        static size_t size(const T& str) noexcept           { return sizeof(T) / sizeof(std::remove_extent_t<T>) - 1; }
        static const char_type* data(const T& str) noexcept { return str; }
    };

    template <typename T>
    struct string_traits_impl<T, typename std::enable_if_t<is_std_encoded_character_string<T>::value
#if _HAS_CXX17
                                                        || is_std_encoded_character_string_view<T>::value
#endif
    >>
    {
        using char_type = typename T::value_type;

        static size_t size(const T& str) noexcept           { return str.size(); }
        static const char_type* data(const T& str) noexcept { return str.data(); }
    };

    template <typename T>
    struct string_traits : string_traits_impl<T> { };


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