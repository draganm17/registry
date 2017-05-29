/** @file */
#pragma once

#include <cstdint>
#include <iterator>
#include <type_traits>

#include <registry/details/string_codec.h>
#include <registry/details/string_traits.h>
#include <registry/details/key_path_utility.h>
#include <registry/types.h>


namespace registry
{
    /*! On 64-bit Windows, portions of the registry entries are stored separately for 32-bit application and 64-bit
    applications and mapped into separate logical registry views using the registry redirector and registry reflection,
    because the 64-bit version of an application may use different registry keys and values than the 32-bit version. 
    These flags enable explicit access to the 64-bit registry view and the 32-bit view, respectively. For more 
    information see: https://msdn.microsoft.com/ru-ru/library/windows/desktop/ms724072 */
    enum class view : uint32_t
    {
        /*! Access a 32-bit key from a 32-bit application or a 64-bit key from a 64-bit application. */
        view_default =  0x00000000,

        /*! Access a 64-bit key from either a 32-bit or 64-bit application. */
        view_32bit =    0x00000200,

        /*! Access a 32-bit key from either a 32-bit or 64-bit application. \n
        Ignored on the 32-bit versions of Windows. */
        view_64bit =    0x00000100
    };


    //------------------------------------------------------------------------------------//
    //                                 class key_path                                     //
    //------------------------------------------------------------------------------------//

    //! Represents a path to a registry key.
    /*!
    An object of class `key_path` represents a path on the Windows registry and contains a key name and a registry view.
    Such an object is concerned only with the lexical and syntactic aspects of a path. The path does not necessarily 
    exist in the registry, and the key name is not necessarily valid.
    
    The key name has the following syntax:
    1. Root key name (optional): a root registry key (such as `HKEY_LOCAL_MACHINE`).
    2. Zero or more of the following:
        - subkey name:    sequence of characters that aren't key separators.
        - key-separators: the backslash character `\`. If this character is repeated, it is treated as a single key 
                          separator: `HKEY_LOCAL_MACHINE\Software` is the same as `HKEY_LOCAL_MACHINE\\\\Software`.
                          // TODO: remove the part about multiple separators ???

    // TODO: something about the view ...
    //       the view is always present evenen if the key name is empty ...

    The path can be traversed element-wise via iterators returned by the `begin()` and `end()` functions, which iterates
    over all components af the key name (key separators are skipped). Each element has the same registry view as the 
    whole path. Calling any non-const member function of a path invalidates all iterators referring to elements of that 
    object.
    */
    class key_path
    {
    public:
        class iterator;
        using const_iterator = iterator;

    public:
        //! Key separator. Always a backslash.
        static constexpr string_type::value_type separator = string_type::value_type('\\');

    private:
        template <typename CharT>
        key_path(const CharT* first, const CharT* last, view view);

        key_path(const string_type::value_type* first, const string_type::value_type* last, view view);

        key_path& do_assign(const string_type::value_type* first, const string_type::value_type* last, view view);

        key_path& do_append(const string_type::value_type* first, const string_type::value_type* last, view view);

        key_path& do_concat(const string_type::value_type*, const string_type::value_type* last, view view);

        key_path& do_replace_leaf_path(const string_type::value_type*, const string_type::value_type* last, view view);

    public:
        //! Constructs a path that identifies an predefined registry key.
        /*!
        @param[in] id - a predefined key identifier.
        
        @return 
            If `id == key_id::unknown`, returns `key_path()`. Otherwise, returns `p`, where `p.key_name()`
            contains the name of the predefined registry key and `p.key_view()` is equal to `view::view_default`.
        */
        static key_path from_key_id(key_id id);

    public:
        //! Default constructor.
        /*!
        @post `key_name().empty()`.
        @post `key_view() == view::view_default`.
        */
        key_path() noexcept = default;

        //! Constructs the path with the copy of the contents of `other`.
        /*!
        @post `*this == other`.
        */
        key_path(const key_path& other) = default;

        /*! \brief
        Constructs the path with the contents of `other` using move semantics. `other` is left in a valid but
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        */
        key_path(key_path&& other) noexcept = default;

        //! Constructs the path from a registry view.
        /*!
        @post `key_name().empty()`.
        @post `key_view() == view`.

        @param[in] view - a registry view.
        */
        explicit key_path(view view);

        //! Constructs the path from a key name and a registry view.
        /*!
        The current path key name is composed as follows: \n
        `name` is traversed element-wise, given that one element can be separated from enother by a sequence of one or
        more key separators. Each element of `name` is appended to the key name preceeding by exactly one key separator,
        except for the first element, which is not preceded by a separator. Key separators that preceed the first or 
        follow the last element of `name` are ignored. If `name` is an empty string or a string that consists entirely
        of key separators, nothing is appended to the key name.

        @post `key_view() == view`.

        @param[in] name - a key name. `Source` should be explicitly convertible to `registry::string_view_type`.
        @param[in] view - a registry view.
        */
        template <typename Source,
                  typename = std::enable_if_t<details::encoding::is_string<Source>::value && 
                                              details::encoding::is_deducible<Source>::value>
        >
        key_path(const Source& name, view view = view::view_default);

        //! Replaces the contents of `*this` with a copy of the contents of `other`.
        /*!
        @post `*this == other`.
        @return `*this`.
        */
        key_path& operator=(const key_path& other) = default;

        /*! \brief
        Replaces the contents of `*this` with those of `other` using move semantics. `other` is left in a valid, but 
        unspecified state. */
        /*!
        @post `*this` has the original value of `other`.
        @return `*this`.
        */
        key_path& operator=(key_path&& other) noexcept = default;

        //! Appends elements to the path with a key separator.
        /*!
        Equivalent to `append(src)`.

        @param[in] src - a path to append. `Source` should be explicitly convertible to `registry::key_path`.
        @return `*this`.
        */
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<key_path, T>::value>
        >
        key_path& operator/=(const Source& src);

        //! Concatenates the current path and `src` without introducing a key separator.
        /*!
        Equivalent to `concat(src)`.

        @param[in] src - a path to append. `Source` should be explicitly convertible to `registry::key_path`.
        @return `*this`.
        */
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<key_path, T>::value>
        >
        key_path& operator+=(const Source& src);

    public:
        //! Returns the name of the key.
        const string_type& key_name() const noexcept;

        //! Returns the registry view of the key.
        view key_view() const noexcept;

        //! Returns the root path of the path.
        /*!
        If `root_key_id() != key_id::unknown`, returns `key_path(*begin())`. Otherwise, returns `key_path(key_view())`.
        */
        key_path root_path() const;

        //! Returns the identifier of the root key.
        /*!
        If `begin() != end()` and `*begin()` identifies a predefined registry key, returns that key identifier.
        Otherwise, returns `key_id::unknown`.
        */
        key_id root_key_id() const noexcept;

        //! Returns the leaf component of the path.
        /*!
        If `begin() != end()`, returns `key_path(*--end())`. Otherwise, returns `key_path(key_view())`.
        */
        key_path leaf_path() const;

        //! Returns the parent of the path.
        /*!
        If `begin() != end()`, returns `p`, where `p` is constructed as if by `key_path p(key_view())` and 
        successively applying `operator/=` for each element in the range `[begin(), --end())`. Otherwise, returns 
        `key_path(key_view())`.
        */
        key_path parent_path() const;

        //! Returns a path relative to the root path.
        /*!
        If `has_root_path()`, returns `p`, where `p` is constructed as if by `key_path p(key_view())` and successively 
        applying `operator/=` for each element in the range `[++begin(), end())`. Otherwise, returns `*this`.
        */
        key_path relative_path() const;

        //! Checks whether `root_path()` has at least one component.
        bool has_root_path() const noexcept;

        //! Checks whether `leaf_path()` has at least one component.
        bool has_leaf_path() const noexcept;

        //! Checks whether `parent_path()` has at least one component.
        bool has_parent_path() const noexcept;

        //! Checks whether `relative_path()` has at least one component.
        bool has_relative_path() const noexcept;

        //! Checks whether the path is absolute.
        /*!
        An absolute path is a path that unambiguously identifies the location of a registry key without reference to
        an additional starting location. The key name of an absolute path always begins with a predefined key identifier.
        */
        bool is_absolute() const noexcept;

        //! Checks whether the path is relative.
        /*!
        Equivalent to `!is_absolute()`.
        */
        bool is_relative() const noexcept;

        //! Compares paths objects.
        /*!
        - if `key_view() < other.key_view()`, `*this` is less than `other`;
        - otherwise if `key_view() > other.key_view()`, `*this` is greater than `other`;
        - otherwise the key name components are compared lexicographically. The comparison is case-insensitive.

        @return
            A value less than 0 if `*this` is less then `other`. \n
            A value equal to 0 if `*this` is equal to `other`.   \n
            A value greater than 0 if `*this` is greater than `other`.
        */
        int compare(const key_path& other) const noexcept;

        //! Returns an iterator to the first component of the path.
        /*!
        If the path has no components, returns `end()`.
        */
        iterator begin() const;

        //! Returns an iterator one past the last component of the path.
        /*!
        Dereferencing this iterator is undefined behavior.
        */
        iterator end() const noexcept;

    public:
        //! Replaces the contents of the path.
        /*!
        @post `*this == key_path(view)`.

        @param[in] view - a registry view.

        @return `*this`.
        */
        key_path& assign(view view);

        //! Replaces the contents of the path.
        /*!
        @post `*this == key_path(name, view)`.

        @param[in] name - a key name string.
        @param[in] view - a registry view.

        @return `*this`.
        */
        template <typename Source,
                  typename = std::enable_if_t<details::encoding::is_string<Source>::value && 
                                              details::encoding::is_deducible<Source>::value>
        >
        key_path& assign(const Source& name, view view = view::view_default);

        //! Appends elements to the path with a key separator.
        /*!
        Establishes the postcondition, as if by applying the following steps:
        - Constructs an object `p` as if by `key_path p(src)`;
        - Appends `separator` to the current path key name, except if any of the following conditions is true:
          - `key_name()` is an empty string;
          - `p.key_name()` is an empty string.
        - Appends `p.key_name()` the to the current path key name;
        - Replaces the current path key view with `p.key_view()`, except if `p.key_view() == view::view_default`.

        @param[in] src - a path to append. `Source` should be explicitly convertible to `registry::key_path`.

        @return `*this`.
        */
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<key_path, T>::value>
        >
        key_path& append(const Source& src);

        //! Concatenates the current path and `src` without introducing a key separator.
        /*!
        Establishes the postcondition, as if by applying the following steps:
        - Constructs an object `p` as if by `key_path p(src)`;
        - Appends `p.key_name()` the  to the current path key name;
        - Replaces the current path key view with `p.key_view()`, except if `p.key_view() == view::view_default`.

        @param[in] src - a path to append. `Source` should be explicitly convertible to `registry::key_path`.

        @return `*this`.
        */
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<key_path, T>::value>
        >
        key_path& concat(const Source& src);

        //! Removes a single leaf component.
        /*!
        If `begin() == end()`, does nothing. \n
        Note that the leaf component of the key name is removed along with the preceding key separator, if present.

        @return `*this`.
        */
        key_path& remove_leaf_path();

        //! Replaces a single leaf component with `src`.
        /*!
        Equivalent to `remove_leaf_path().append(src)`.

        @pre `has_leaf_path()`.

        @param[in] src - `Source` should be explicitly convertible to `registry::key_path`.

        @return `*this`.
        */
        template <typename Source, 
                  typename = std::enable_if_t<std::is_constructible<key_path, T>::value>
        >
        key_path& replace_leaf_path(const Source& src);

        // TODO: ... 
        //       to be able to set the key viewy to the default value
        // void replace_key_view(view);

        //! Swaps the contents of `*this` and `other`.
        void swap(key_path& other) noexcept;

    private:
        view         m_view = view::view_default;
        string_type  m_name;

    };

    //------------------------------------------------------------------------------------//
    //                          class key_path::iterator                                  //
    //------------------------------------------------------------------------------------//

    //! A constant BidirectionalIterator with a value_type of registry::key_path.
    class key_path::iterator
    {
        friend class key_path;

    public:
        using value_type =        key_path;
        using difference_type =   std::ptrdiff_t;
        using pointer =           const value_type*;
        using reference =         const value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

    public:
        //! Checks whether `*this` is equal to `rhs`.
        bool operator==(const iterator& rhs) const noexcept;

        //! Checks whether `*this` is not equal to `rhs`.
        bool operator!=(const iterator& rhs) const noexcept;

        //! Accesses the pointed-to `registry::key_path`.
        /*!
        @pre `*this` is a valid iterator and not the end iterator.
        @return Value of the `registry::key_path` referred to by this iterator.
        */
        reference operator*() const noexcept;

        //! Accesses the pointed-to `registry::key_path`.
        /*!
        @pre `*this` is a valid iterator and not the end iterator.
        @return Pointer to the `registry::key_path` referred to by this iterator.
        */
        pointer operator->() const noexcept;

    public:
        //! Advances the iterator to the next entry, then returns `*this`.
        /*!
        @pre `*this` is a valid iterator and not the end iterator.
        */
        iterator& operator++();

        //! Makes a copy of `*this`, calls operator++(), then returns the copy.
        /*!
        @pre `*this` is a valid iterator and not the end iterator.
        */
        iterator operator++(int);

        //! Decrements the iterator to the previous entry, then returns `*this`.
        /*!
        @pre `*this` is a valid iterator and not the begin iterator.
        */
        iterator& operator--();

        //! Makes a copy of `*this`, calls operator--(), then returns the copy.
        /*!
        @pre `*this` is a valid iterator and not the begin iterator.
        */
        iterator operator--(int);

    public:
        //! Swaps the contents of `*this` and `other`.
        void swap(iterator& other) noexcept;

    private:
        key_path                    m_element;
        details::key_name_iterator  m_name_it;
        const key_path*             m_path_ptr;
    };

    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    //! Appends `lhs` to `rhs` with a key separator.
    /*!
    Equivalent to `key_path(lhs).append(rhs)`.

    @param[in] lhs - the left hand side.
    @param[in] rhs - the rights hand side. `Source` should be explicitly convertible to `registry::key_path`.
    */
    template <typename Source, 
              typename = std::enable_if_t<std::is_constructible<key_path, T>::value>
    >
    key_path operator/(const key_path& lhs, const Source& rhs);

    //! Checks whether `lhs` is equal to `rhs`. Equivalent to `lhs.compare(rhs) == 0`.
    bool operator==(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is not equal to `rhs`. Equivalent to `lhs.compare(rhs) != 0`.
    bool operator!=(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is less than `rhs`. Equivalent to `lhs.compare(rhs) < 0`.
    bool operator<(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is greater than `rhs`. Equivalent to `lhs.compare(rhs) > 0`.
    bool operator>(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is less than or equal to `rhs`. Equivalent to `lhs.compare(rhs) <= 0`.
    bool operator<=(const key_path& lhs, const key_path& rhs) noexcept;

    //! Checks whether `lhs` is greater than or equal to `rhs`. Equivalent to `lhs.compare(rhs) >= 0`.
    bool operator>=(const key_path& lhs, const key_path& rhs) noexcept;

    //! Calculates a hash value for a `key_path` object.
    /*!
    @return A hash value such that if for two paths, `p1 == p2` then `hash_value(p1) == hash_value(p2)`.
    */
    size_t hash_value(const key_path& path) noexcept;

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key_path& lhs, key_path& rhs) noexcept;

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

    template <typename CharT>
    key_path::key_path(const CharT* first, const CharT* last, view view)
    {
        using namespace details;
        const auto name = encoding::codec<encoding::deduce_t<CharT>>().decode(first, last);
        key_path(encoding::native_encoding_type{}, name.data(), name.data() + name.size(), view).swap(*this);
    }

    template <typename Source, typename = std::enable_if_t<details::encoding::is_string<Source>::value && 
                                                           details::encoding::is_deducible<Source>::value>>
    inline key_path::key_path(const Source& name, view view)
    : key_path(details::encoding::string_traits<Source>::data(name),
               details::encoding::string_traits<Source>::data(name) + details::encoding::string_traits<Source>::size(name),
               view)
    { }

    template <typename Source, typename = std::enable_if_t<std::is_constructible<key_path, T>::value>>
    inline key_path& key_path::operator/=(const Source& src) { return append(src); }

    template <typename Source, typename = std::enable_if_t<std::is_constructible<key_path, T>::value>>
    inline key_path& key_path::operator+=(const Source& src) { return concat(src); }

    template <typename Source, typename = std::enable_if_t<details::encoding::is_string<Source>::value && 
                                                           details::encoding::is_deducible<Source>::value>>
    key_path& key_path::assign(const Source& name, view view)
    {
        using ST = details::encoding::string_traits<Source>;
        using Codec = details::encoding::codec<details::encoding::deduce_t<Source>>;

        auto dec_name = Codec().decode(ST::data(name), ST::data(name) + ST::size(name));
        return do_assign(dec_name.data(), dec_name.data() + dec_name.size(), view);

        // TODO: optimization for the case when the string is already in the native encoding.
    }

    template <typename Source, typename = std::enable_if_t<std::is_constructible<key_path, T>::value>>
    inline key_path& key_path::append(const Source& src)
    {
        const key_path p(src);
        return do_append(p.key_name().data(), p.key_name().data() + p.key_name().size(), p.key_view());
        // TODO: optimization : avoid to construct a new path
    }

    template <typename Source, typename = std::enable_if_t<std::is_constructible<key_path, T>::value>>
    inline key_path& key_path::concat(const Source& src)
    {
        const key_path p(src);
        return do_concat(p.key_name().data(), p.key_name().data() + p.key_name().size(), p.key_view());
        // TODO: optimization : avoid to construct a new path
    }

    template <typename Source, typename = std::enable_if_t<std::is_constructible<key_path, T>::value>>
    inline key_path& key_path::replace_leaf_path(const Source& src)
    {
        assert(has_leaf_path());
        const key_path p(src);
        return do_replace_leaf_path(p.key_name().data(), p.key_name().data() + p.key_name().size(), p.key_view());
        // TODO: optimization : avoid to construct a new path
    }

    template <typename Source, typename = std::enable_if_t<std::is_constructible<key_path, T>::value>>
    inline key_path operator/(const key_path& lhs, const Source& rhs) { return key_path(lhs).append(rhs); }

    inline bool operator==(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) == 0; }

    inline bool operator!=(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) != 0; }

    inline bool operator<(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) < 0; }

    inline bool operator>(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) > 0; }

    inline bool operator<=(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) <= 0; }

    inline bool operator>=(const key_path& lhs, const key_path& rhs) noexcept { return lhs.compare(rhs) >= 0; }

    inline void swap(key_path& lhs, key_path& rhs) noexcept { lhs.swap(rhs); }

} // namespace registry