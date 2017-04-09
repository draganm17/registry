#include <cassert>
#include <Windows.h>

#include <registry/exception.h>
#include <registry/details/utils.impl.h>
#include <registry/key_handle.h>
#include <registry/operations.h>


#define RETURN_RESULT(ec, result)          \
    {                                      \
        if (&ec != &throws()) ec.clear();  \
        return result;                     \
    }


namespace registry {

//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

bool create_key(const key& key, std::error_code& ec)
{
    std::error_code ec2;
    bool result = false;
    registry::key base_key = key, subkey;
    auto handle = open(base_key, access_rights::create_sub_key, ec2);

    while ((!ec2 || ec2.value() == ERROR_FILE_NOT_FOUND) && base_key.has_parent_key()) {
        subkey = base_key.leaf_key().append(subkey.name());
        handle = open(base_key.remove_leaf(), access_rights::create_sub_key, ec2);
    }

    if (!ec2 && (result = handle.create_key(subkey, access_rights::query_value, ec).second, !ec2)) {
        RETURN_RESULT(ec, result);
    }
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key), result;
}

bool equivalent(const key& key1, const key& key2, std::error_code& ec)
{
    std::error_code ec2;
    bool result = false;
    key_handle handle1, handle2;
    if ((handle1 = open(key1, access_rights::query_value, ec2), !ec2) &&
        (handle2 = open(key2, access_rights::query_value, ec2), !ec2) &&
        (result = handle1.equivalent(handle2, ec2), !ec2))
    {
        RETURN_RESULT(ec, result);
    }
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key1, key2), result;
}

bool exists(const key& key, std::error_code& ec)
{
    std::error_code ec2;
    open(key, access_rights::query_value, ec2);

    if (!ec2) RETURN_RESULT(ec, true);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key), false;
}

bool exists(const key& key, string_view_type value_name, std::error_code& ec)
{
    std::error_code ec2;
    bool result = false;
    const auto handle = open(key, access_rights::query_value, ec2);

    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, result);
    if (!ec2 && (result = handle.exists(value_name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key, registry::key(), value_name), false;
}

key_info info(const key& key, key_info_mask mask, std::error_code& ec)
{
    constexpr key_info invalid_info{ uint32_t(-1), uint32_t(-1), uint32_t(-1), 
                                     uint32_t(-1), uint32_t(-1), key_time_type::min() };

    key_info info;
    std::error_code ec2;
    const auto handle = open(key, access_rights::query_value, ec2);

    if (!ec2 && (info = handle.info(mask, ec2), !ec2)) RETURN_RESULT(ec, info);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key), invalid_info;
}

value read_value(const key& key, string_view_type value_name, std::error_code& ec)
{
    value result;
    std::error_code ec2;
    const auto handle = open(key, access_rights::query_value, ec2);

    if (!ec2 && (result = handle.read_value(value_name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key, registry::key(), value_name), result;
}

bool remove(const key& key, std::error_code& ec)
{
    // TODO: key open rights does not affect the delete operation - 
    //       so, maybe I should 'open' just the root key and not the parent key ???

    std::error_code ec2;
    bool result = false;
    const auto handle = open(key.parent_key(), access_rights::query_value /* TODO: ??? */, ec2);

    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, result);
    if (!ec2 && (result = handle.remove(key.leaf_key(), ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key), result;

    // TODO: check if the key has a parent ???
}

bool remove(const key& key, string_view_type value_name, std::error_code& ec)
{
    std::error_code ec2;
    bool result = false;
    const auto handle = open(key, access_rights::set_value, ec2);

    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, result);
    if (!ec2 && (result = handle.remove(value_name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, key, registry::key(), value_name), result;
}

std::uintmax_t remove_all(const key& key, std::error_code& ec)
{
    // TODO: ...
    return 0;

    //auto keys_deleted = remove_all_inside(key, ec);
    //if (!ec) {
    //    keys_deleted += remove(key, ec) ? 1 : 0;
    //}
    //return ec ? static_cast<std::uintmax_t>(-1) : keys_deleted;
}

space_info space(std::error_code& ec)
{
    space_info info;
    constexpr space_info invalid_info{ uint32_t(-1), uint32_t(-1) };

    const BOOL success = GetSystemRegistryQuota(reinterpret_cast<DWORD*>(&info.capacity), 
                                                reinterpret_cast<DWORD*>(&info.size));

    if (success) RETURN_RESULT(ec, info);
    const std::error_code ec2(GetLastError(), std::system_category());
    return details::set_or_throw(&ec, ec2, __FUNCTION__), invalid_info;
}

void write_value(const key& key, string_view_type value_name, const value& value, std::error_code& ec)
{
    std::error_code ec2;
    const auto handle = open(key, access_rights::set_value, ec2);

    if (!ec2) RETURN_RESULT(ec, void());
    details::set_or_throw(&ec, ec2, __FUNCTION__, key, registry::key(), value_name);
}

}  // namespace registry