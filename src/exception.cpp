#include <registry/exception.h>
#include <registry/key_path.h>
#include <registry/value_name.h>


namespace registry {

//-------------------------------------------------------------------------------------------//
//                                  class registry_error                                     //
//-------------------------------------------------------------------------------------------//

struct registry_error::storage
{
    key_path               path1;
    key_path               path2;
    registry::value_name   value_name;
};

registry_error::registry_error(std::error_code ec, const std::string& msg)
: std::system_error(ec, msg)
{ }

registry_error::registry_error(std::error_code ec, const std::string& msg, const key_path& path1)
: std::system_error(ec, msg)
, m_info(std::make_shared<storage>(storage{ path1 }))
{ }

registry_error::registry_error(std::error_code ec, const std::string& msg,
                               const key_path& path1, const key_path& path2)
: std::system_error(ec, msg)
, m_info(std::make_shared<storage>(storage{ path1, path2 }))
{ }

registry_error::registry_error(
                std::error_code ec, const std::string& msg,
                const key_path& path1, const key_path& path2, const registry::value_name& name)
: std::system_error(ec, msg)
, m_info(std::make_shared<storage>(storage{ path1, path2, name }))
{ }

const key_path& registry_error::path1() const noexcept
{
    static const key_path empty_path;
    return m_info ? m_info->path1 : empty_path;
}

const key_path& registry_error::path2() const noexcept
{
    static const key_path empty_path;
    return m_info ? m_info->path2 : empty_path;
}

const value_name& registry_error::value_name() const noexcept
{
    static const registry::value_name empty_value_name;
    return m_info ? m_info->value_name : empty_value_name;
}

}  // namespace registry