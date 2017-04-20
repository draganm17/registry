#include <cassert>
#include <Windows.h>

#include <registry/exception.h>
#include <registry/details/utils.impl.h>
#include <registry/key_path.h>
#include <registry/key_handle.h>
#include <registry/operations.h>


namespace registry {

//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

bool create_key(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    key_path base_path = path, subkey_path;
    key_handle handle(base_path, access_rights::create_sub_key, ec2);

    if (!ec2) RETURN_RESULT(ec, false);
    while (ec2.value() == ERROR_FILE_NOT_FOUND && base_path.has_parent_key()) {
        subkey_path = base_path.leaf_key().append(subkey_path.key_name());
        handle = key_handle(base_path.remove_leaf_key(), access_rights::create_sub_key, ec2);
    }

    bool result;
    if (!ec2 && (result = handle.create_key(subkey_path, access_rights::all_access, ec2).second, !ec2)) {
        RETURN_RESULT(ec, result);
    }
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

bool equivalent(const key_path& path1, const key_path& path2, std::error_code& ec)
{
    bool result;
    std::error_code ec2;
    key_handle handle1, handle2;
    if ((handle1 = key_handle(path1, access_rights::query_value, ec2), !ec2) &&
        (handle2 = key_handle(path2, access_rights::query_value, ec2), !ec2) &&
        (result = handle1.equivalent(handle2, ec2), !ec2))
    {
        RETURN_RESULT(ec, result);
    }
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path1, path2), false;
}

bool key_exists(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    key_handle(path, access_rights::query_value, ec2);

    if (!ec2) RETURN_RESULT(ec, true);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

bool value_exists(const key_path& path, string_view_type value_name, std::error_code& ec)
{
    std::error_code ec2;
    const key_handle handle(path, access_rights::query_value, ec2);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    bool result;
    if (!ec2 && (result = handle.value_exists(value_name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path, key_path(), value_name), false;
}

key_info info(const key_path& path, key_info_mask mask, std::error_code& ec)
{
    constexpr key_info invalid_info{ uint32_t(-1), uint32_t(-1), uint32_t(-1), 
                                     uint32_t(-1), uint32_t(-1), key_time_type::min() };

    std::error_code ec2;
    const key_handle handle(path, access_rights::query_value, ec2);

    key_info info;
    if (!ec2 && (info = handle.info(mask, ec2), !ec2)) RETURN_RESULT(ec, info);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), invalid_info;
}

value read_value(const key_path& path, string_view_type value_name, std::error_code& ec)
{
    std::error_code ec2;
    const key_handle handle(path, access_rights::query_value, ec2);

    value result;
    if (!ec2 && (result = handle.read_value(value_name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path, key_path(), value_name), result;
}

bool remove(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    // NOTE: key open rights does not affect the delete operation.
    const key_handle handle(path.parent_key(), access_rights::query_value, ec2);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    bool result;
    if (!ec2 && (result = handle.remove(path.leaf_key(), ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), false;
}

bool remove(const key_path& path, string_view_type value_name, std::error_code& ec)
{
    std::error_code ec2;
    const key_handle handle(path, access_rights::set_value, ec2);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, false);

    bool result;
    if (!ec2 && (result = handle.remove(value_name, ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path, key_path(), value_name), false;
}

uint32_t remove_all(const key_path& path, std::error_code& ec)
{
    std::error_code ec2;
    // NOTE: key open rights does not affect the delete operation.
    const key_handle handle(path.parent_key(), access_rights::query_value, ec2);
    if (ec2.value() == ERROR_FILE_NOT_FOUND) RETURN_RESULT(ec, 0);

    uint32_t result;
    if (!ec2 && (result = handle.remove_all(path.leaf_key(), ec2), !ec2)) RETURN_RESULT(ec, result);
    return details::set_or_throw(&ec, ec2, __FUNCTION__, path), static_cast<uint32_t>(-1);
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

void write_value(const key_path& path, string_view_type value_name, const value& value, std::error_code& ec)
{
    std::error_code ec2;
    const key_handle handle(path, access_rights::set_value, ec2);

    if (!ec2 && (handle.write_value(value_name, value), !ec2)) RETURN_RESULT(ec, VOID);
    details::set_or_throw(&ec, ec2, __FUNCTION__, path, key_path(), value_name);
}

}  // namespace registry