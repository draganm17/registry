#pragma once

#include <locale>
#include <string>
#include <type_traits>


namespace registry {
namespace details {

    struct system_narrow_encoding
    {
        using char_type = char;
    };

    struct system_wide_encoding
    {
        using char_type = wchar_t;
    };

    struct utf8_encoding
    {
        using char_type = char;
    };

    //struct utf16_encoding
    //{
    //    using char_type = char16_t;
    //};

    //struct utf32_encoding
    //{
    //    using char_type = char32_t;
    //};

    using native_encoding_type = system_wide_encoding;


    template <typename T, typename = void>
    struct string_traits { };

    template <typename T>
    struct string_traits<T*, typename std::enable_if_t<std::is_same<std::remove_cv_t<T>, char>::value     ||
                                                       std::is_same<std::remove_cv_t<T>, wchar_t>::value
                                                     /*std::is_same<std::remove_cv_t<T>, char16_t>::value ||
                                                       std::is_same<std::remove_cv_t<T>, char32_t>::value */>>
    {
    private:
        static auto get_default_encoding(char)     -> system_narrow_encoding;
        static auto get_default_encoding(wchar_t)  -> system_wide_encoding;
        //static auto get_default_encoding(char16_t) -> utf16_encoding;
        //static auto get_default_encoding(char32_t) -> utf32_encoding;

    public:
        using char_type = std::remove_cv_t<T>;
        using default_encoding_type = decltype(get_default_encoding(char_type{}));

        static size_t size(const char_type* str) noexcept           { return std::char_traits<char_type>::length(str); }
        static const char_type* data(const char_type* str) noexcept { return str; }
    };

    template <typename CharT, typename Traits, typename Alloc>
    struct string_traits<std::basic_string<CharT, Traits, Alloc>,
                         typename std::enable_if_t<sizeof(typename string_traits<CharT*>::char_type)>>
    {
        using char_type = typename string_traits<CharT*>::char_type;
        using default_encoding_type = typename string_traits<CharT*>::default_encoding_type;

        static size_t size(const std::basic_string<CharT, Traits, Alloc>& str) noexcept           { return str.size(); }
        static const char_type* data(const std::basic_string<CharT, Traits, Alloc>& str) noexcept { return str.data(); }
    };

    template <typename CharT, typename Traits>
    struct string_traits<std::basic_string_view<CharT, Traits>,
                         typename std::enable_if_t<sizeof(typename string_traits<CharT*>::char_type)>>
    {
        using char_type = typename string_traits<CharT*>::char_type;
        using default_encoding_type = typename string_traits<CharT*>::default_encoding_type;

        static size_t size(const std::basic_string_view<CharT, Traits>& str) noexcept           { return str.size(); }
        static const char_type* data(const std::basic_string_view<CharT, Traits>& str) noexcept { return str.data(); }
    };


    template <typename Encoding,
              typename EncAlloc = std::allocator<typename Encoding::char_type>,
              typename DecAlloc = std::allocator<typename native_encoding_type::char_type>
    >
    class codec_base
    {
    public:
        using encoded_string_type = std::basic_string<typename Encoding::char_type,
                                                      std::char_traits<typename Encoding::char_type>, EncAlloc>;

        using decoded_string_type = std::basic_string<typename native_encoding_type::char_type,
                                                      std::char_traits<typename native_encoding_type::char_type>, DecAlloc>;

    protected:
        codec_base() = default;

        codec_base(const codec_base&) = delete;

        codec_base& operator=(const codec_base&) = delete;

        ~codec_base() = default;
    };

    template <typename Encoding,
              typename EncAlloc = std::allocator<typename Encoding::char_type>,
              typename DecAlloc = std::allocator<typename native_encoding_type::char_type>
    >
    class codec /* : public codec_base<system_narrow_encoding, EncAlloc, DecAlloc> */
    {
        // TODO: ...
         static_assert(sizeof(Encoding) == 0, "Encoding not supported.");
    };

    template <typename EncAlloc, typename DecAlloc>
    class codec<system_narrow_encoding, EncAlloc, DecAlloc> 
    : public codec_base<system_narrow_encoding, EncAlloc, DecAlloc>
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

    template <typename EncAlloc, typename DecAlloc>
    class codec<system_wide_encoding, EncAlloc, DecAlloc> 
    : public codec_base<system_wide_encoding, EncAlloc, DecAlloc>
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

}} // namespace registry::details