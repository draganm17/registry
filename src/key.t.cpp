#include <array>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/exception.h>
#include <registry/key.h>
#include <registry/key_path.h>
#include <registry/operations.h>

using namespace registry;


TEST(Key, Construct)
{
    // default constructor
    {
        key k;
        EXPECT_TRUE(!k.is_open());
    }

    // key(key_id)
    {
        static const auto test = [](key_id id)
        {
            const key k = id;
            if (id != key_id::unknown) {
                EXPECT_TRUE(k.is_open());
                EXPECT_TRUE(k.rights() == access_rights::unknown);
                EXPECT_TRUE(k.native_handle() == reinterpret_cast<key::native_handle_type>(id));
            } else {
                EXPECT_TRUE(!k.is_open());
            }
        };

        test(key_id::classes_root);
        test(key_id::current_user);
        test(key_id::local_machine);
        test(key_id::users);
        test(key_id::performance_data);
        test(key_id::performance_text);
        test(key_id::performance_nlstext);
        test(key_id::current_config);
        test(key_id::current_user_local_settings);
        test(key_id::unknown);
    }

    // key(open_only_tag, const key_path&, access_rights)
    // key(open_only_tag, const key_path&, access_rights, std::error_code&)
    {
        std::error_code ec;

        // open a predefined key
        const key_path p1 = key_path::from_key_id(key_id::current_user);
        const key k1(open_only_tag(), p1, access_rights::read);
        EXPECT_TRUE(k1.is_open()                       &&
                    k1.rights() == access_rights::read &&
                    k1.native_handle() != key::native_handle_type());
        //
        const key k2(open_only_tag(), p1, access_rights::read, ec);
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(k2.is_open()                       &&
                    k2.rights() == access_rights::read &&
                    k2.native_handle() != key::native_handle_type());

        // open an existing key
        const key_path p2 = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry";
        const key k3(open_only_tag(), p2, access_rights::read);
        EXPECT_TRUE(k3.is_open()                       &&
                    k3.rights() == access_rights::read &&
                    k3.native_handle() != key::native_handle_type());
        //
        const key k4(open_only_tag(), p2, access_rights::read, ec);
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(k4.is_open()                       &&
                    k4.rights() == access_rights::read &&
                    k4.native_handle() != key::native_handle_type());

        // try open an non-existing key
        const key_path p3 = "HKEY_CURRENT_USER\\SOFTWARE\\non_existent";
        EXPECT_THROW(const key k5(open_only_tag(), p3, access_rights::read), registry_error);
        //
        const key k6(open_only_tag(), p3, access_rights::read, ec);
        EXPECT_TRUE(ec && !k6.is_open());
    }

    // key(open_or_create_tag, const key_path&, access_rights)
    // key(open_or_create_tag, const key_path&, access_rights, std::error_code&)
    {
        // TODO: ...
    }

    // key(open_or_create_tag, const key_path&, access_rights, bool&)
    // key(open_or_create_tag, const key_path&, access_rights, bool&, std::error_code& ec)
    {
        // TODO: ...
    }
}

TEST(Key, OperationsOnRegistry)
{
    // key::key_exists(const key_path&)
    // key::key_exists(const key_path&, std::error_code&)
    {
        std::error_code ec;
        const key k(open_only_tag(), "HKEY_CURRENT_USER\\SOFTWARE", access_rights::read);

        EXPECT_TRUE(k.key_exists("libregistry") == true);
        EXPECT_TRUE(k.key_exists("libregistry", ec) == true && !ec);

        EXPECT_TRUE(k.key_exists("non_existent") == false);
        EXPECT_TRUE(k.key_exists("non_existent", ec) == false && !ec);
    }

    // key::value_exists(string_view_type)
    // key::value_exists(string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key k(open_only_tag(), "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read", access_rights::query_value);

        EXPECT_TRUE(k.value_exists("val_01") == true);
        EXPECT_TRUE(k.value_exists("val_01", ec) == true && !ec);

        EXPECT_TRUE(k.value_exists("non_existent") == false);
        EXPECT_TRUE(k.value_exists("non_existent", ec) == false && !ec);
    }

    // key::info()
    // key::info(std::error_code&)
    {
        static const auto test_mask = [](const key& k, key_info_mask mask)
        {
            std::error_code ec;
            const key_info i1 = k.info(mask), i2 = k.info(mask, ec);
            EXPECT_TRUE(!ec);
            EXPECT_TRUE(
                i1.subkeys             == i2.subkeys             && i1.values              == i2.values              &&
                i1.max_subkey_size     == i2.max_subkey_size     && i1.max_value_name_size == i2.max_value_name_size &&
                i1.max_value_data_size == i2.max_value_data_size && i1.last_write_time     == i2.last_write_time);

            const bool r_subkeys =             (mask & key_info_mask::read_subkeys)             != key_info_mask::none;
            const bool r_values =              (mask & key_info_mask::read_values)              != key_info_mask::none;
            const bool r_max_subkey_size =     (mask & key_info_mask::read_max_subkey_size)     != key_info_mask::none;
            const bool r_max_value_name_size = (mask & key_info_mask::read_max_value_name_size) != key_info_mask::none;
            const bool r_max_value_data_size = (mask & key_info_mask::read_max_value_data_size) != key_info_mask::none;
            const bool r_last_write_time =     (mask & key_info_mask::read_last_write_time)     != key_info_mask::none;

            EXPECT_TRUE((r_subkeys             && i1.subkeys             == uint32_t(-1)) || true);
            EXPECT_TRUE((r_values              && i1.values              == uint32_t(-1)) || true);
            EXPECT_TRUE((r_max_subkey_size     && i1.max_subkey_size     == uint32_t(-1)) || true);
            EXPECT_TRUE((r_max_value_name_size && i1.max_value_name_size == uint32_t(-1)) || true);
            EXPECT_TRUE((r_max_value_data_size && i1.max_value_data_size == uint32_t(-1)) || true);
            EXPECT_TRUE((r_last_write_time     && i1.last_write_time     == key_time_type::min()) || true);
        };

        std::error_code ec;
        const key k(open_only_tag(), "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read", access_rights::query_value);

        test_mask(k, key_info_mask::none);
        test_mask(k, key_info_mask::read_subkeys);
        test_mask(k, key_info_mask::read_values);
        test_mask(k, key_info_mask::read_max_subkey_size);
        test_mask(k, key_info_mask::read_max_value_name_size);
        test_mask(k, key_info_mask::read_max_value_data_size);
        test_mask(k, key_info_mask::read_last_write_time);
        test_mask(k, key_info_mask::all);
    }

    // key::read_value(string_view_type)
    // key::read_value(string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key k(open_only_tag(), "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read", access_rights::query_value);
            
        auto v01  = k.read_value("val_01");
        auto v01a = k.read_value("val_01", ec);
        EXPECT_TRUE(!ec && v01 == v01a);
        EXPECT_TRUE(v01.type() == value_type::none && v01.size() == 0);

        auto v02  = k.read_value("val_02");
        auto v02a = k.read_value("val_02", ec);
        EXPECT_TRUE(!ec && v02 == v02a);
        EXPECT_TRUE(v02.type() == value_type::sz && v02.to_string() == "42");

        auto v03  = k.read_value("val_03");
        auto v03a = k.read_value("val_03", ec);
        EXPECT_TRUE(!ec && v03 == v03a);
        EXPECT_TRUE(v03.type() == value_type::expand_sz && v03.to_string() == "42");

        auto v04  = k.read_value("val_04");
        auto v04a = k.read_value("val_04", ec);
        EXPECT_TRUE(!ec && v04 == v04a);
        EXPECT_TRUE(v04.type() == value_type::binary &&
                    v04.size() == 2                  &&
                    memcmp(std::array<char, 2>{ 4, 2 }.data(), v04.data(), v04.size()) == 0);


        auto v05  = k.read_value("val_05");
        auto v05a = k.read_value("val_05", ec);
        EXPECT_TRUE(!ec && v05 == v05a);
        EXPECT_TRUE(v05.type() == value_type::dword && v05.to_uint32() == 42);

        auto v06  = k.read_value("val_06");
        auto v06a = k.read_value("val_06", ec);
        EXPECT_TRUE(!ec && v06 == v06a);
        EXPECT_TRUE(v06.type() == value_type::dword_big_endian && v06.to_uint32() == 42);

        auto v07  = k.read_value("val_07");
        auto v07a = k.read_value("val_07", ec);
        EXPECT_TRUE(!ec && v07 == v07a);
        EXPECT_TRUE(v07.type() == value_type::link && v07.to_string() == "42");

        auto v08  = k.read_value("val_08");
        auto v08a = k.read_value("val_08", ec);
        EXPECT_TRUE(!ec && v08 == v08a);
        EXPECT_TRUE(v08.type() == value_type::multi_sz && (v08.to_strings() == 
                    std::vector<string_type>{ "42", "42" }));

        auto v09  = k.read_value("val_09");
        auto v09a = k.read_value("val_09", ec);
        EXPECT_TRUE(!ec && v09 == v09a);
        EXPECT_TRUE(v09.type() == value_type::qword && v09.to_uint64() == 42);

        EXPECT_THROW(k.read_value("non_existent"), registry_error);
        //
        auto v10 = k.read_value("non_existent", ec);
        EXPECT_TRUE(ec && v10 == value());
    }

    // key::create_key(const key_path&, access_rights)
    // key::create_key(const key_path&, access_rights, std::error_code&)
    {
        std::error_code ec;
        key k(open_or_create_tag(), "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write", access_rights::all_access);

        // create new keys (without subkeys)
        const key_path p1 = "new_key_1";
        ASSERT_TRUE(!k.key_exists(p1));                                     // check that the keys does not exist
        const auto ret1 = k.create_key(p1, access_rights::read);            // create the first key
        EXPECT_TRUE(k.key_exists(p1));                                      // check operation success
        EXPECT_TRUE(ret1.second == true                      &&
                    ret1.first.is_open()                     && 
                    ret1.first.rights() == access_rights::read);            // check the result
        //
        const key_path p2 = "new_key_2";
        ASSERT_TRUE(!k.key_exists(p2));                                     // check that the keys does not exist
        const auto ret2 = k.create_key(p2, access_rights::write, ec);       // create the second key
        EXPECT_TRUE(!ec && k.key_exists(p2));                               // check operation success
        EXPECT_TRUE(ret2.second == true                      &&
                    ret2.first.is_open()                     && 
                    ret2.first.rights() == access_rights::write);           // check the result

        // create new keys (with subkeys)
        const key_path p3 = "new_key_3\\Inner1\\Inner2";
        ASSERT_TRUE(!k.key_exists(p3));                                     // check that the keys does not exist
        const auto ret3 = k.create_key(p3, access_rights::all_access);      // create the first key
        EXPECT_TRUE(k.key_exists(p3));                                      // check operation success
        EXPECT_TRUE(ret3.second == true                      &&
                    ret3.first.is_open()                     &&
                    ret3.first.rights() == access_rights::all_access);      // check the result
        //
        const key_path p4 = "new_key_4\\Inner1\\Inner2";
        ASSERT_TRUE(!k.key_exists(p4));                                     // check that the keys does not exist
        const auto ret4 = k.create_key(p4, access_rights::all_access, ec);  // create the second key
        EXPECT_TRUE(!ec && k.key_exists(p4));                               // check operation success
        EXPECT_TRUE(ret4.second == true                      &&
                    ret4.first.is_open()                     &&
                    ret4.first.rights() == access_rights::all_access);      // check the result

        // open an existing key
        const auto ret5 = k.create_key(p1, access_rights::all_access);      // open the key
        EXPECT_TRUE(ret5.second == false                     &&
                    ret5.first.is_open()                     &&
                    ret5.first.rights() == access_rights::all_access);      // check the result
        //
        // open an existing key
        const auto ret6 = k.create_key(p1, access_rights::all_access, ec);  // open the key
        EXPECT_TRUE(!ec);                                                   // check operation success
        EXPECT_TRUE(ret6.second == false                     &&
                    ret6.first.is_open()                     &&
                    ret6.first.rights() == access_rights::all_access);      // check the result

        // open the same key
        const auto ret7 = k.create_key({}, access_rights::all_access);      // open the key
        EXPECT_TRUE(ret7.second == false          &&
                    ret7.first.is_open()          &&
                    ret7.first.rights() == access_rights::all_access);      // check the result
        //
        const auto ret8 = k.create_key({}, access_rights::all_access, ec);  // open the key
        EXPECT_TRUE(!ec);                                                   // check operation success
        EXPECT_TRUE(ret8.second == false          &&
                    ret8.first.is_open()          &&
                    ret8.first.rights() == access_rights::all_access);      // check the result
    }

    // key::write_value(string_view_type, const value&)
    // key::write_value(string_view_type, const value&, std::error_code&)
    {
        std::error_code ec;
        const std::array<uint8_t, 2> bytes{ 4, 2};
        key k(open_only_tag(), 
              "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write",
              access_rights::set_value | access_rights::query_value);
        
        const value v01(none_value_tag());
        k.write_value("val_01", v01);
        EXPECT_TRUE((k.write_value("val_01a", v01, ec), !ec));
        EXPECT_TRUE(k.read_value("val_01") == v01 && k.read_value("val_01a") == v01);

        const value v02(sz_value_tag(), "42");
        k.write_value("val_02", v02);
        EXPECT_TRUE((k.write_value("val_02a", v02, ec), !ec));
        EXPECT_TRUE(k.read_value("val_02") == v02 && k.read_value("val_02a") == v02);

        const value v03(expand_sz_value_tag(), "42");
        k.write_value("val_03", v03);
        EXPECT_TRUE((k.write_value("val_03a", v03, ec), !ec));
        EXPECT_TRUE(k.read_value("val_03") == v03 && k.read_value("val_03a") == v03);

        const value v04(binary_value_tag(), bytes.data(), bytes.size());
        k.write_value("val_04", v04);
        EXPECT_TRUE((k.write_value("val_04a", v04, ec), !ec));
        EXPECT_TRUE(k.read_value("val_04") == v04 && k.read_value("val_04a") == v04);

        const value v05(dword_value_tag(), 42);
        k.write_value("val_05", v05);
        EXPECT_TRUE((k.write_value("val_05a", v05, ec), !ec));
        EXPECT_TRUE(k.read_value("val_05") == v05 && k.read_value("val_05a") == v05);

        const value v06(dword_big_endian_value_tag(), 42);
        k.write_value("val_06", v06);
        EXPECT_TRUE((k.write_value("val_06a", v06, ec), !ec));
        EXPECT_TRUE(k.read_value("val_06") == v06 && k.read_value("val_06a") == v06);

        const value v07(link_value_tag(), "42");
        k.write_value("val_07", v07);
        EXPECT_TRUE((k.write_value("val_07a", v07, ec), !ec));
        EXPECT_TRUE(k.read_value("val_07") == v07 && k.read_value("val_07a") == v07);

        const value v08(multi_sz_value_tag(), { "42", "42" });
        k.write_value("val_08", v08);
        EXPECT_TRUE((k.write_value("val_08a", v08, ec), !ec));
        EXPECT_TRUE(k.read_value("val_08") == v08 && k.read_value("val_08a") == v08);

        const value v09(qword_value_tag(), 42);
        k.write_value("val_09", v09);
        EXPECT_TRUE((k.write_value("val_09a", v09, ec), !ec));
        EXPECT_TRUE(k.read_value("val_09") == v09 && k.read_value("val_09a") == v09);  
    }

    // key::remove_value(string_view_type)
    // key::remove_value(string_view_type, std::error_code& ec)
    {
        std::error_code ec;
        key k(open_only_tag(), "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write", access_rights::set_value | 
                                                                                  access_rights::query_value);

        ASSERT_TRUE(!k.value_exists("non_existing") &&
                     k.value_exists("val_01")       && 
                     k.value_exists("val_02"));

        // remove an non-existing value
        EXPECT_TRUE(k.remove_value("non_existing") == false);
        EXPECT_TRUE(k.remove_value("non_existing", ec) == false && !ec);

        // remove an existing value
        EXPECT_TRUE(k.remove_value("val_01") == true && !k.value_exists("val_01"));
        EXPECT_TRUE(k.remove_value("val_02", ec) == true && !ec && !k.value_exists("val_02"));
    }

    // key::remove_key(string_view_type)
    // key::remove_key(string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write";
        key k(open_only_tag(), p, access_rights::all_access);

        // remove an non-existing key
        const key_path p1 = "non_existing";
        ASSERT_TRUE(!k.key_exists(p1));
        EXPECT_TRUE(k.remove_key(p1) == false);
        EXPECT_TRUE(k.remove_key(p1, ec) == false && !ec);

        // remove an empty key (with no subkeys)
        const key_path p2 = "new_key_1";
        ASSERT_TRUE(k.key_exists(p2) && info(key_path(p).append(p2)).subkeys == 0);
        EXPECT_TRUE(k.remove_key(p2) == true && !k.key_exists(p2));
        //
        const key_path p3 = "new_key_2";
        ASSERT_TRUE(k.key_exists(p3) && info(key_path(p).append(p3)).subkeys == 0);
        EXPECT_TRUE(k.remove_key(p3, ec) == true && !ec && !k.key_exists(p3));
    }

    // key::remove_keys(string_view_type)
    // key::remove_keys(string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key_path p = "HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write";
        key k(open_only_tag(), p, access_rights::all_access);

        // remove an non-existing key
        const key_path p1 = "non_existing";
        ASSERT_TRUE(!k.key_exists(p1));                                             // check that the key does not exist
        EXPECT_TRUE(k.remove_keys(p1) == 0);                                        // remove the key and check the result
        //
        const key_path p2 = "non_existing";
        ASSERT_TRUE(!k.key_exists(p2));                                             // check that the key does not exist
        EXPECT_TRUE(k.remove_keys(p2, ec) == 0 && !ec);                             // remove the key and check the result

        // remove an non-empty key (which have subkeys)
        const key_path p3 = "new_key_3";
        ASSERT_TRUE(k.key_exists(p3) && info(key_path(p).append(p3)).subkeys > 0);  // check that the key exists and have subkeys
        EXPECT_TRUE(k.remove_keys(p3) == 3 && !k.key_exists(p3));                   // remove the key and check the result
        //
        const key_path p4 = "new_key_4";
        ASSERT_TRUE(k.key_exists(p4) && info(key_path(p).append(p4)).subkeys > 0);  // check that the key exists and have subkeys
        EXPECT_TRUE(k.remove_keys(p4, ec) == 3 && !ec && !k.key_exists(p4));        // remove the key and check the result

        // clean-up
        registry::remove_keys(p);
    }

    // key::equivalent(const key_path&)
    // key::equivalent(const key_path&, std::error_code& ec)
    {
        // TODO: ...
    }

    // key::equivalent(const key&)
    // key::equivalent(const key&, std::error_code& ec)
    {
        // TODO: ...
    }
}

TEST(Key, Close)
{
    std::error_code ec;

    // close a default-constructed key
    key k1;
    k1.close();
    EXPECT_TRUE(!k1.is_open());
    //
    key k2;
    k2.close(ec);
    EXPECT_TRUE(!ec, !k2.is_open());

    // close an predefined key
    key k3 = key_id::current_user;
    k3.close();
    EXPECT_TRUE(!k3.is_open());
    //
    key k4 = key_id::current_user;
    k4.close(ec);
    EXPECT_TRUE(!ec, !k3.is_open());

    // close a regular key
    key k5(open_only_tag(), "HKEY_CURRENT_USER\\SOFTWARE\\libregistry", access_rights::read);
    k5.close();
    EXPECT_TRUE(!k5.is_open());
    //
    key k6(open_only_tag(), "HKEY_CURRENT_USER\\SOFTWARE\\libregistry", access_rights::read);
    k6.close(ec);
    EXPECT_TRUE(!ec && !k6.is_open());
}

TEST(Key, Swap)
{
    // TODO: ...
}