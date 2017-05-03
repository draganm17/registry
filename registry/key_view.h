/** @file */
#pragma once

#include <exception>
#include <system_error>
#include <type_traits>


namespace registry
{
    class key;

    //------------------------------------------------------------------------------------//
    //                                 class key_view                                     //
    //------------------------------------------------------------------------------------//

    //! TODO: ...
    /*!
    TODO: ...
    */
    class key_view
    {
    public:
        key_view(const key& key) noexcept;

    public:
        const key& key() const noexcept;

    public:
        void swap(key_view& other) noexcept;

    private:
        const registry::key* m_ptr;
    };

    //------------------------------------------------------------------------------------//
    //                             NON-MEMBER FUNCTIONS                                   //
    //------------------------------------------------------------------------------------//

    //! Swaps the contents of `lhs` and `rhs`.
    void swap(key_view& lhs, key_view& rhs) noexcept;

    //------------------------------------------------------------------------------------//
    //                              INLINE DEFINITIONS                                    //
    //------------------------------------------------------------------------------------//

    inline void swap(key_view& lhs, key_view& rhs) noexcept { lhs.swap(rhs); }

} // namespace registry