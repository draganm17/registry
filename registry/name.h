/** @file */
#pragma once

#include <locale>
#include <string>
#include <string_view>

#include <registry/details/encoding.h>


namespace registry
{
    //-------------------------------------------------------------------------------------------//
    //                                       class name                                          //
    //-------------------------------------------------------------------------------------------//

    class name
    {
    public:
        using value_type = wchar_t;

    public:
        //! Default constructor.
        /*!
        //  @post `empty()`.
        */
        name() noexcept = default;

        //! Constructs the name with the copy of the contents of `other`.
        /*!
        //  @post `*this == other`.
        */
        name(const name& other) = default;

        /*! \brief
        //  Constructs the name with the contents of `other` using move semantics. `other` is left
        //  in a valid but unspecified state. */
        /*!
        //  @post `*this` has the original value of `other`.
        */
        name(name&& other) noexcept = default;

        /*! \brief
        //  Constructs the name with the contents of `source` using move semantics. `source` is left
        //  in a valid but unspecified state. */
        /*!
        //  @post `this->value()` has the original value of `source`.
        */
        name(std::basic_string<value_type>&& source) noexcept;

        //! TODO: ...
        template <typename CharT>
        name(const CharT* source, const std::locale& loc = std::locale());

        //! TODO: ...
        template <typename CharT>
        name(const std::basic_string<CharT>& source, const std::locale& loc = std::locale());

        //! TODO: ...
        template <typename CharT>
        name(std::basic_string_view<CharT> source, const std::locale& loc = std::locale());

        //! TODO: ...
        template <typename InputIt>
        name(InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        //  @post `*this == other`.
        //
        //  @return `*this`.
        */
        name& operator=(const name& other) = default;

        /*! \brief
        //  Replaces the contents of `*this` with those of `other` using move semantics. `other` is
        //  left in a valid, but unspecified state. */
        /*!
        //  @post `*this` has the original value of `other`.
        //
        //  @return `*this`.
        */
        name& operator=(name&& other) noexcept = default;

        //! Calls `value() = source`.
        /*!
        //  @return `*this`.
        */
        name& operator=(const value_type* source);

        // TODO: ...
        template <typename CharT>
        name& operator=(const CharT* source);

        //! Calls `value() = source`.
        /*!
        //  @return `*this`.
        */
        name& operator=(const std::basic_string<value_type>& source);

        //! Calls `value() = std::move(source)`.
        /*!
        //  @return `*this`.
        */
        name& operator=(std::basic_string<value_type>&& source) noexcept;

        // TODO: ...
        template <typename CharT>
        name& operator=(const std::basic_string<CharT>& source);

        //! Calls `value() = source`.
        /*!
        //  @return `*this`.
        */
        name& operator=(std::basic_string_view<value_type> source);

        // TODO: ...
        template <typename CharT>
        name& operator=(std::basic_string_view<CharT> source);

        /*! \brief
        //  Returns a `std::basic_string<value_type>`, constructed 
        //  as if by `std::basic_string<value_type>(c_str(), size())`. */
        operator std::basic_string<value_type>() const;

        /*! \brief
        //  Returns a `std::basic_string_view<value_type>`, constructed
        //  as if by `std::basic_string_view<value_type>(c_str(), size())`. */
        operator std::basic_string_view<value_type>() const noexcept;

    public:
        //! Returns `value().empty()`.
        bool empty() const noexcept;

        //! Returns `value().data()`.
        value_type* data() noexcept;

        //! Returns `value().data()`.
        const value_type* data() const noexcept;

        //! Returns `value().size()`.
        size_t size() const noexcept;

        //! Returns a reference to the contained string.
        constexpr std::basic_string<value_type>& value() & noexcept;

        //! Returns a reference to the contained string.
        constexpr const std::basic_string<value_type>& value() const & noexcept;

        //! Returns a reference to the contained string.
        constexpr std::basic_string<value_type>&& value() && noexcept;

        //! Returns a reference to the contained string.
        constexpr const std::basic_string<value_type>&& value() const && noexcept;

        //! TODO: ...
        std::string string(const std::locale& loc = std::locale()) const;

        //! TODO: ...
        std::wstring wstring(const std::locale& loc = std::locale()) const; // TODO: rm loc ???

        //! TODO: ...
        std::string u8string(const std::locale& loc = std::locale()) const; // TODO: rm loc ???

        //! TODO: ...
        std::u16string u16string(const std::locale& loc = std::locale()) const; // TODO: rm loc ???

        //! TODO: ...
        std::u32string u32string(const std::locale& loc = std::locale()) const; // TODO: rm loc ???

        //! Compares names objects.
        /*! Names are compared lexicographically. The comparison is case-insensitive.
        //
        //  @return
        //      A value less than 0 if `*this` is less then `other`. \n
        //      A value equal to 0 if `*this` is equal to `other`.   \n
        //      A value greater than 0 if `*this` is greater than `other`.
        */
        int compare(const name& name) const noexcept;

    public:
        //! Calls `value().assign(source)`.
        /*!
        //  @return `*this`.
        */
        name& assign(const value_type* source);

        //! TODO: ...
        template <typename CharT>
        name& assign(const CharT* source, const std::locale& loc = std::locale());

        //! Calls `value().assign(source)`.
        /*!
        //  @return `*this`.
        */
        name& assign(const std::basic_string<value_type>& source);

        //! Calls `value().assign(std::move(source))`.
        /*!
        //  @return `*this`.
        */
        name& assign(std::basic_string<value_type>&& source) noexcept;

        //! TODO: ...
        template <typename CharT>
        name& assign(const std::basic_string<CharT>& source, const std::locale& loc = std::locale());

        //! Calls `value().assign(source)`.
        /*!
        //  @return `*this`.
        */
        name& assign(std::basic_string_view<value_type> source);

        //! TODO: ...
        template <typename CharT>
        name& assign(std::basic_string_view<CharT> source, const std::locale& loc = std::locale());

        //! TODO: ...
        /*!
        //  @return `*this`.
        */
        template <typename InputIt>
        name& assign(InputIt first, InputIt last, const std::locale& loc = std::locale());

        //! Calls `value().clear()`.
        void clear() noexcept;

        //! Swaps the contents of `*this` and `other`.
        void swap(name& other) noexcept;

    private:
        std::basic_string<value_type> m_value;
    };


    //-------------------------------------------------------------------------------------------//
    //                                   NON-MEMBER FUNCTIONS                                    //
    //-------------------------------------------------------------------------------------------//

    // TODO: ...
    template <typename Source>
    name u8name(const Source& source); // TODO: locale ???

    // TODO: ...
    template <typename InputIt>
    name u8name(InputIt first, InputIt last); // TODO: locale ???

    //! Checks whether `lhs` is equal to `rhs`. Equivalent to `lhs.compare(rhs) == 0`.
    bool operator==(const name& lhs, const name& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`. Equivalent to `lhs.compare(rhs) != 0`.
    bool operator!=(const name& lhs, const name& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`. Equivalent to `lhs.compare(rhs) < 0`.
    bool operator<(const name& lhs, const name& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`. Equivalent to `lhs.compare(rhs) > 0`.
    bool operator>(const name& lhs, const name& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`. Equivalent to `lhs.compare(rhs) <= 0`.
    bool operator<=(const name& lhs, const name& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`. Equivalent to `lhs.compare(rhs) >= 0`.
    bool operator>=(const name& lhs, const name& rhs) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(name& lhs, name& rhs) noexcept;


    //-------------------------------------------------------------------------------------------//
    //                                    INLINE DEFINITIONS                                     //
    //-------------------------------------------------------------------------------------------//

    template <typename CharT>
    inline name::name(const CharT* source, const std::locale& loc)
    : name(details::to_native(source, loc))
    { }

    template <typename CharT>
    inline name::name(const std::basic_string<CharT>& source, const std::locale& loc)
    : name(details::to_native(source, loc))
    { }

    template <typename CharT>
    inline name::name(std::basic_string_view<CharT> source, const std::locale& loc)
    : name(details::to_native(source, loc))
    { }

    template <typename InputIt>
    inline name::name(InputIt first, InputIt last, const std::locale& loc)
    : name(details::to_native(first, last, loc))
    { }

    template <typename CharT>
    inline name& name::operator=(const CharT* source)
    {
        return assign(source);
    }

    template <typename CharT>
    inline name& name::operator=(const std::basic_string<CharT>& source)
    {
        return assign(source);
    }

    template <typename CharT>
    inline name& name::operator=(std::basic_string_view<CharT> source)
    {
        return assign(source);
    }

    template <typename CharT>
    inline name& name::assign(const CharT* source, const std::locale& loc)
    {
        return assign(details::to_native(source, loc));
    }

    template <typename CharT>
    inline name& name::assign(const std::basic_string<CharT>& source, const std::locale& loc)
    {
        return assign(details::to_native(source, loc));
    }

    template <typename CharT>
    inline name& name::assign(std::basic_string_view<CharT> source, const std::locale& loc)
    {
        return assign(details::to_native(source, loc));
    }

    template <typename InputIt>
    inline name& name::assign(InputIt first, InputIt last, const std::locale& loc)
    {
        return assign(details::to_native(first, last, loc));
    }

    template <typename Source>
    inline name u8name(const Source& source)
    {
        // TODO: ...
        throw 0;
    }

    template <typename InputIt>
    inline name u8name(InputIt first, InputIt last)
    {
        // TODO: ...
        throw 0;
    }

    inline bool operator==(const name& lhs, const name& rhs) noexcept
    {
        return lhs.compare(rhs) == 0;
    }

    inline bool operator!=(const name& lhs, const name& rhs) noexcept
    {
        return lhs.compare(rhs) != 0;
    }

    inline bool operator<(const name& lhs, const name& rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    inline bool operator>(const name& lhs, const name& rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    inline bool operator<=(const name& lhs, const name& rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    inline bool operator>=(const name& lhs, const name& rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }

    inline void swap(name& lhs, name& rhs) noexcept
    {
        lhs.swap(rhs);
    }

} // namespace registry


namespace std
{
    //-------------------------------------------------------------------------------------------//
    //                               class hash<registry::name>                                  //
    //-------------------------------------------------------------------------------------------//

    //! std::hash specialization for `registry::name`.
    template <>
    struct hash<registry::name>
    {
        //! Calculates a hash value for a `name` object.
        /*!
        //  @return A hash value such that if for two values, `v1 == v2`
        //          then `hash<registry::name>()(v1) == hash<registry::name>()(v2)`.
        */
        size_t operator()(const registry::name& val) const noexcept;
    };

} // namespace std