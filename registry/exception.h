/** @file */
#pragma once

#include <exception>
#include <memory>
#include <string>
#include <system_error>

#include <registry/types.h>


namespace registry
{
    class key_path;


    //-------------------------------------------------------------------------------------------//
    //                                  class registry_error                                     //
    //-------------------------------------------------------------------------------------------//

    /*! \brief 
    //  Defines an exception object that is thrown on failure by the overloads of registry library
    //  functions not having an argument of type `std::error_code&`.
    */
    class registry_error : public std::system_error
    {
    public:
        //! Constructs the error object.
        /*! The explanatory string is set to `msg`, error code is set to `ec`, the first key path,
        //  the second key path and the value name are default-constructed.
        */
        registry_error(std::error_code ec, const std::string& msg);

        //! Constructs the error object.
        /*! The explanatory string is set to `msg`, error code is set to `ec`, the first key path 
        //  is set to `path1`, the second key path and the value name are default-constructed. 
        */
        registry_error(std::error_code ec, const std::string& msg, 
                       const key_path& path1);

        //! Constructs the error object.
        /*! The explanatory string is set to `msg`, error code is set to `ec`, the first key path 
        //  is set to `path1`, the second key path is set to `path2`, the value name is 
        //  default-constructed.
        */
        registry_error(std::error_code ec, const std::string& msg, 
                       const key_path& path1, const key_path& path2);
        
        //! Constructs the error object.
        /*! The explanatory string is set to `msg`, error code is set to `ec`, the first key path 
        //  is set to `path1`, the second key path is set to `path2`, the value name is set to 
        //  `value_name`.
        */
        registry_error(std::error_code ec, const std::string& msg,
                       const key_path& path1, const key_path& path2, string_view_type value_name);

    public:
        //! Returns the first paths that was stored in the exception object.
        const key_path& path1() const noexcept;

        //! Returns the second paths that was stored in the exception object.
        const key_path& path2() const noexcept;

        //! Returns the value name that was stored in the exception object.
        const string_type& value_name() const noexcept;

    private:
        struct storage;
        std::shared_ptr<storage> m_info;
    };

} // namespace registry