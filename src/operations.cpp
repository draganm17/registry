#include <cassert>
#include <Windows.h>

#include <registry/key_handle.h>
#include <registry/operations.h>


namespace registry {

//------------------------------------------------------------------------------------//
//                             NON-MEMBER FUNCTIONS                                   //
//------------------------------------------------------------------------------------//

bool create_key(const key& key)
{
    std::error_code ec;
    decltype(auto) res = create_key(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

bool create_key(const key& key, std::error_code& ec)
{
    ec.clear();
    registry::key base_key = key, subkey;
    key_handle handle = open(base_key, access_rights::create_sub_key, ec);

    while ((!ec || ec.value() == ERROR_FILE_NOT_FOUND) && base_key.has_parent_key()) {
        subkey = base_key.leaf_key().append(subkey.name());
        handle = open(base_key.remove_leaf(), access_rights::create_sub_key, ec);
    }
    return !ec ? handle.create_key(subkey, access_rights::query_value, ec).second : false;
}

bool equivalent(const key& key1, const key& key2)
{
    std::error_code ec;
    decltype(auto) res = equivalent(key1, key2, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key1, key2);
    return res;
}

bool equivalent(const key& key1, const key& key2, std::error_code& ec)
{
    ec.clear();
    auto handle1 = open(key1, access_rights::query_value, ec);
    auto handle2 = !ec ? open(key2, access_rights::query_value, ec) : key_handle();

    return !ec ? handle1.equivalent(handle2, ec) : false;
}

bool exists(const key& key)
{
   std::error_code ec;
   decltype(auto) res = exists(key, ec);
   if (ec) throw registry_error(ec, __FUNCTION__, key);
   return res;
}

bool exists(const key& key, std::error_code& ec)
{
    ec.clear();
    open(key, access_rights::query_value, ec);
    return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), false)
                                                : (!ec ? true : false);
}

bool exists(const key& key, string_view_type value_name)
{
    std::error_code ec;
    decltype(auto) res = exists(key, value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, value_name);
    return res;
}

bool exists(const key& key, string_view_type value_name, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::query_value, ec);
    return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), false)
                                                : (!ec ? handle.exists(value_name, ec) : false);
}

key_info info(const key& key, key_info_mask mask)
{
    std::error_code ec;
    decltype(auto) res = info(key, mask, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

key_info info(const key& key, key_info_mask mask, std::error_code& ec)
{
    ec.clear();
    constexpr key_info invalid_info{ uint32_t(-1), uint32_t(-1), uint32_t(-1), 
                                     uint32_t(-1), uint32_t(-1), key_time_type::min() };

    auto handle = open(key, access_rights::query_value, ec);
    return !ec ? handle.info(mask, ec) : invalid_info;
}

value read_value(const key& key, string_view_type value_name)
{
    std::error_code ec;
    decltype(auto) res = read_value(key, value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, value_name);
    return res;
}

value read_value(const key& key, string_view_type value_name, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::query_value, ec);
    return !ec ? handle.read_value(value_name, ec) : value();
}

bool remove(const key& key)
{
    std::error_code ec;
    decltype(auto) res = remove(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

bool remove(const key& key, std::error_code& ec)
{
    // TODO: key open rights does not affect the delete operation - 
    //       so, maybe I should 'open' just the root key and not the parent key ???

    ec.clear();
    auto handle = open(key.parent_key(), access_rights::query_value /* TODO: ??? */, ec);

    return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), false)
                                                : (!ec ? handle.remove(key.leaf_key(), ec) : false);

    // TODO: check if the key has a parent ???
}

bool remove(const key& key, string_view_type value_name)
{
    std::error_code ec;
    decltype(auto) res = remove(key, value_name, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, value_name);
    return res;
}

bool remove(const key& key, string_view_type value_name, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::set_value, ec);
    return (ec.value() == ERROR_FILE_NOT_FOUND) ? (ec.clear(), false)
                                                : (!ec ? handle.remove(value_name, ec) : false);
}

std::uintmax_t remove_all(const key& key)
{
    std::error_code ec;
    decltype(auto) res = remove_all(key, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key);
    return res;
}

std::uintmax_t remove_all(const key& key, std::error_code& ec)
{
    // TODO: ...
    return 0;

    ec.clear();

    //auto keys_deleted = remove_all_inside(key, ec);
    //if (!ec) {
    //    keys_deleted += remove(key, ec) ? 1 : 0;
    //}
    //return ec ? static_cast<std::uintmax_t>(-1) : keys_deleted;
}

space_info space()
{
    std::error_code ec;
    decltype(auto) res = space(ec);
    if (ec) throw registry_error(ec, __FUNCTION__);
    return res;
}

space_info space(std::error_code& ec)
{
    ec.clear();
    space_info info;
    constexpr space_info invalid_info{ uint32_t(-1), uint32_t(-1) };

    BOOL success = GetSystemRegistryQuota(reinterpret_cast<DWORD*>(&info.capacity), 
                                          reinterpret_cast<DWORD*>(&info.size));

    return success ? info : (ec = std::error_code(GetLastError(), std::system_category()), invalid_info);
}

void write_value(const key& key, string_view_type value_name, const value& value)
{
    std::error_code ec;
    write_value(key, value_name, value, ec);
    if (ec) throw registry_error(ec, __FUNCTION__, key, {}, value_name);
}

void write_value(const key& key, string_view_type value_name, const value& value, std::error_code& ec)
{
    ec.clear();
    auto handle = open(key, access_rights::set_value, ec);
    if (!ec) handle.write_value(value_name, value, ec);
}

}  // namespace registry