#include <registry/key.h>
#include <registry/types.h>


namespace registry {

//------------------------------------------------------------------------------------//
//                             class registry_error                                   //
//------------------------------------------------------------------------------------//

struct registry_error::storage
{
    key          key1;
    key          key2;
    string_type  value_name;
};

registry_error::registry_error(std::error_code ec, const std::string& msg)
    : std::system_error(ec, msg)
{ }

registry_error::registry_error(std::error_code ec, const std::string& msg,
                               const key& key1)
    : std::system_error(ec, msg)
    , m_info(std::make_shared<storage>(storage{ key1 }))
{ }

registry_error::registry_error(std::error_code ec, const std::string& msg,
                               const key& key1, const key& key2)
    : std::system_error(ec, msg)
    , m_info(std::make_shared<storage>(storage{ key1, key2 }))
{ }

registry_error::registry_error(std::error_code ec, const std::string& msg,
                               const key& key1, const key& key2, string_view_type value_name)
    : std::system_error(ec, msg)
    , m_info(std::make_shared<storage>(storage{ key1, key2, static_cast<string_type>(value_name) }))
{ }

const key& registry_error::key1() const noexcept
{
    static const key empty_key;
    return m_info ? m_info->key1 : empty_key;
}

const key& registry_error::key2() const noexcept
{
    static const key empty_key;
    return m_info ? m_info->key2 : empty_key;
}

const string_type& registry_error::value_name() const noexcept
{
    static const string_type empty_value_name;
    return m_info ? m_info->value_name : empty_value_name;
}

}  // namespace registry