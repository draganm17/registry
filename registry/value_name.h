/** @file */
#pragma once

#include <locale>
#include <string>

#include <registry/details/encoding.h>


namespace registry
{
    //-------------------------------------------------------------------------------------------//
    //                                    class value_name                                       //
    //-------------------------------------------------------------------------------------------//

    class value_name
    {
    public:
        value_name() noexcept = default;

        value_name(const std::wstring& source);

        value_name(std::wstring&& source) noexcept;

        template <typename Source>
        value_name(const Source& source, const std::locale& loc = std::locale(""));

        template <typename InputIt>
        value_name(InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        value_name(const value_name& other) = default;

        value_name(value_name&& other) noexcept = default;

        value_name& operator=(const std::wstring& source);

        value_name& operator=(std::wstring&& source) noexcept;

        template <typename Source>
        value_name& operator=(const Source& source);

        value_name& operator=(const value_name& other) = default;

        value_name& operator=(value_name&& other) noexcept = default;

    public:
        bool empty() const noexcept; // TODO: ???

        const wchar_t* c_str() const noexcept;

        size_t size() const noexcept; // TODO: ???

        const std::wstring& get() const noexcept;

        std::string string(const std::locale& loc = std::locale(""));

        std::wstring wstring();

        operator std::wstring() const;

        int compare(const value_name& name) const noexcept;
        
        int compare(const std::wstring& str) const;  // TODO: ???
        
        int compare(std::wstring_view str) const;  // TODO: ???
        
        int compare(const wchar_t* s) const;  // TODO: ???

    public:
        value_name& assign(const std::wstring& source);

        value_name& assign(std::wstring&& source) noexcept;

        template <typename Source>
        value_name& assign(const Source& source, const std::locale& loc = std::locale(""));

        template <typename InputIt>
        value_name& assign(InputIt first, InputIt last, const std::locale& loc = std::locale(""));

        void clear() noexcept;

        void swap(value_name& other) noexcept;

    private:
        std::wstring m_name;
    };


    //-------------------------------------------------------------------------------------------//
    //                                   NON-MEMBER FUNCTIONS                                    //
    //-------------------------------------------------------------------------------------------//

    //! Checks whether `lhs` is equal to `rhs`. Equivalent to `lhs.compare(rhs) == 0`.
    bool operator==(const value_name& lhs, const value_name& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`. Equivalent to `lhs.compare(rhs) != 0`.
    bool operator!=(const value_name& lhs, const value_name& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`. Equivalent to `lhs.compare(rhs) < 0`.
    bool operator<(const value_name& lhs, const value_name& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`. Equivalent to `lhs.compare(rhs) > 0`.
    bool operator>(const value_name& lhs, const value_name& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`. Equivalent to `lhs.compare(rhs) <= 0`.
    bool operator<=(const value_name& lhs, const value_name& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`. Equivalent to `lhs.compare(rhs) >= 0`.
    bool operator>=(const value_name& lhs, const value_name& rhs) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(value_name& lhs, value_name& rhs) noexcept;


    //-------------------------------------------------------------------------------------------//
    //                                    INLINE DEFINITIONS                                     //
    //-------------------------------------------------------------------------------------------//

    template <typename Source>
    inline value_name::value_name(const Source& source, const std::locale& loc)
    : value_name(details::to_native(source, loc))
    { }

    template <typename InputIt>
    inline value_name::value_name(InputIt first, InputIt last, const std::locale& loc)
    : value_name(details::to_native(first, last, loc))
    { }

    template <typename Source>
    inline value_name& value_name::operator=(const Source& source)
    {
        return assign(details::to_native(source));
    }

    template <typename Source>
    inline value_name& value_name::assign(const Source& source, const std::locale& loc)
    {
        return assign(details::to_native(source, loc));
    }

    template <typename InputIt>
    inline value_name& value_name::assign(InputIt first, InputIt last, const std::locale& loc)
    {
        return assign(details::to_native(first, last, loc));
    }

    inline bool operator==(const value_name& lhs, const value_name& rhs) noexcept
    {
        return lhs.compare(rhs) == 0;
    }

    inline bool operator!=(const value_name& lhs, const value_name& rhs) noexcept
    {
        return lhs.compare(rhs) != 0;
    }

    inline bool operator<(const value_name& lhs, const value_name& rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    inline bool operator>(const value_name& lhs, const value_name& rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    inline bool operator<=(const value_name& lhs, const value_name& rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    inline bool operator>=(const value_name& lhs, const value_name& rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }

    inline void swap(value_name& lhs, value_name& rhs) noexcept
    {
        lhs.swap(rhs);
    }

} // namespace registry


namespace std
{
    //-------------------------------------------------------------------------------------------//
    //                            class hash<registry::value_name>                               //
    //-------------------------------------------------------------------------------------------//

    //! std::hash specialization for `registry::value_name`.
    template <>
    struct hash<registry::value_name>
    {
        //! Calculates a hash value for a `value_name` object.
        /*!
        //  @return A hash value such that if for two values, `v1 == v2`
        //          then `hash<registry::value_name>()(v1) == hash<registry::value_name>()(v2)`.
        */
        size_t operator()(const registry::value_name& val) const noexcept;
    };

} // namespace std