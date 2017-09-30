#include <cassert>
#include <Windows.h>

#include <registry/exception.h>
#include <registry/details/common_utility.impl.h>
#include <registry/key.h>
#include <registry/key_path.h>
#include <registry/name.h>
#include <registry/operations.h>


namespace registry {

//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

bool create_key(const key_path& path, std::error_code& ec)
{
    bool was_created;
    std::error_code ec2;
    key(open_or_create_tag{}, path, access_rights::read, was_created, ec2);

    if (!ec2) RETURN_RESULT(ec, was_created);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

bool equivalent(const key_path& path1, const key_path& path2, std::error_code& ec)
{
    bool result;
    key key1, key2;
    std::error_code ec2;
    if ((key1 = key(open_only_tag{}, path1, access_rights::query_value, ec2), !ec2) &&
        (key2 = key(open_only_tag{}, path2, access_rights::query_value, ec2), !ec2) &&
        (result = key1.equivalent(key2, ec2), !ec2))
    {
        RETURN_RESULT(ec, result);
    }
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path1, path2), false;
}

key_info info(const key_path& path, key_info_mask mask, std::error_code& ec)
{
    constexpr key_info invalid_info{ uint32_t(-1), uint32_t(-1), uint32_t(-1), 
                                     uint32_t(-1), uint32_t(-1), key_time_type::min() };

    std::error_code ec2;
    const key key(open_only_tag{}, path, access_rights::query_value, ec2);

    key_info info;
    if (!ec2 && (info = key.info(mask, ec2), !ec2)) RETURN_RESULT(ec, info);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), invalid_info;
}

bool key_exists(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    key(open_only_tag{}, path, access_rights::read, ec2);

    if (!ec2) RETURN_RESULT(ec, true);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

value read_value(const key_path& path, const name& name, std::error_code& ec)
{
    std::error_code ec2;
    const key key(open_only_tag{}, path, access_rights::query_value, ec2);

    value result;
    if (!ec2 && (result = key.read_value(name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path, key_path(), name), result;
}

bool remove_key(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    // NOTE: key open rights does not affect the delete operation.
    key key(open_only_tag{}, path.parent_path(), access_rights::read, ec2);

    bool result;
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);
    if (!ec2 && (result = key.remove_key(path.leaf_path(), ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

uint32_t remove_keys(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    // NOTE: key open rights does not affect the delete operation.
    key key(open_only_tag{}, path.parent_path(), access_rights::read, ec2);

    uint32_t result;
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, 0);
    if (!ec2 && (result = key.remove_keys(path.leaf_path(), ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), static_cast<uint32_t>(-1);
}

bool remove_value(const key_path& path, const name& name, std::error_code& ec)
{
    std::error_code ec2;
    key key(open_only_tag{}, path, access_rights::set_value, ec2);

    bool result;
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);
    if (!ec2 && (result = key.remove_value(name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path, key_path(), name), false;
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

bool value_exists(const key_path& path, const name& name, std::error_code& ec)
{
    std::error_code ec2;
    const key key(open_only_tag{}, path, access_rights::query_value, ec2);

    bool result;
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);
    if (!ec2 && (result = key.value_exists(name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path, key_path(), name), false;
}

void write_value(const key_path& path, const name& name, const value& value, std::error_code& ec)
{
    std::error_code ec2;
    key key(open_only_tag{}, path, access_rights::set_value, ec2);

    if (!ec2 && (key.write_value(name, value), !ec2)) RETURN_RESULT(ec, VOID);
    details::set_or_throw(&ec, ec2, __FUNCTION__, path, key_path(), name);
}

}  // namespace registry