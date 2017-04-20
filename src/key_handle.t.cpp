#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/exception.h>
#include <registry/key_handle.h>
#include <registry/operations.h>

using namespace registry;


TEST(KeyHandle, Construct)
{
    // default constructor
    {
        key_handle h;
        EXPECT_TRUE(!h.is_open());
    }

    // key_handle(key_id)
    {
        // TODO: ...
    }

    // key_handle(key_id)
    {
        // TODO: ...
    }

    // key_handle(const registry::key_path&, access_rights)
    // key_handle(const registry::key_path&, access_rights, std::error_code&)
    {
        std::error_code ec;
        const key_path p1 = key_path::from_key_id(key_id::current_user);
        const key_path p2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry");
        const key_path p3 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\non_existent");

        // open a predefined key
        const key_handle h1(p1, access_rights::query_value);
        EXPECT_TRUE(h1.is_open());
        EXPECT_TRUE(h1.path() == p1);
        EXPECT_TRUE(h1.rights() == access_rights::query_value);
        EXPECT_TRUE(h1.native_handle() != key_handle::native_handle_type{});

        const key_handle h2(p1, access_rights::query_value, ec);
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(h2.is_open());
        EXPECT_TRUE(h2.path() == p1);
        EXPECT_TRUE(h2.rights() == access_rights::query_value);
        EXPECT_TRUE(h2.native_handle() != key_handle::native_handle_type{});

        // open a regular key
        const key_handle h3(p2, access_rights::query_value);
        EXPECT_TRUE(h3.is_open());
        EXPECT_TRUE(h3.path() == p2);
        EXPECT_TRUE(h3.rights() == access_rights::query_value);
        EXPECT_TRUE(h3.native_handle() != key_handle::native_handle_type{});

        const key_handle h4(p2, access_rights::query_value, ec);
        EXPECT_TRUE(!ec);
        EXPECT_TRUE(h4.is_open());
        EXPECT_TRUE(h4.path() == p2);
        EXPECT_TRUE(h4.rights() == access_rights::query_value);
        EXPECT_TRUE(h4.native_handle() != key_handle::native_handle_type{});

        // try open an non-existing key
        int exceptions = 0;
        try {
            const key_handle h5(p3, access_rights::query_value);
        } catch (const registry_error& e) {
            ++exceptions;
        }
        EXPECT_TRUE(exceptions == 1);

        const key_handle h6(p3, access_rights::query_value, ec);
        EXPECT_TRUE(ec && !h6.is_open());
    }
}

TEST(KeyHandle, GettersAndQueries)
{
    static const auto test = [](key_id id)
    {
        const key_handle h = id;
        if (id != key_id::unknown) {
            EXPECT_TRUE(h.is_open());
            EXPECT_TRUE(h.path() == key_path::from_key_id(id));
            EXPECT_TRUE(h.rights() == access_rights::unknown);
            EXPECT_TRUE(h.native_handle() == reinterpret_cast<key_handle::native_handle_type>(id));
        } else {
            EXPECT_TRUE(!h.is_open());
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

TEST(KeyHandle, OperationsOnRegistry)
{
    // key_handle::value_exists(string_view_type)
    // key_handle::value_exists(string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key_handle h(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), access_rights::query_value);

        EXPECT_TRUE(h.value_exists(TEXT("val_01")) == true);
        EXPECT_TRUE(h.value_exists(TEXT("val_01"), ec) == true && !ec);

        EXPECT_TRUE(h.value_exists(TEXT("non_existent")) == false);
        EXPECT_TRUE(h.value_exists(TEXT("non_existent"), ec) == false && !ec);
    }

    // key_handle::info()
    // key_handle::info(std::error_code&)
    {
        static const auto test_mask = [](const key_handle& h, key_info_mask mask)
        {
            std::error_code ec;
            const key_info i1 = h.info(mask), i2 = h.info(mask, ec);
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
        const key_handle h(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), access_rights::query_value);

        test_mask(h, key_info_mask::none);
        test_mask(h, key_info_mask::read_subkeys);
        test_mask(h, key_info_mask::read_values);
        test_mask(h, key_info_mask::read_max_subkey_size);
        test_mask(h, key_info_mask::read_max_value_name_size);
        test_mask(h, key_info_mask::read_max_value_data_size);
        test_mask(h, key_info_mask::read_last_write_time);
        test_mask(h, key_info_mask::all);
    }

    // key_handle::read_value(string_view_type)
    // key_handle::read_value(string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key_handle h(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), access_rights::query_value);
            
        auto v01  = h.read_value(TEXT("val_01"));
        auto v01a = h.read_value(TEXT("val_01"), ec);
        EXPECT_TRUE(!ec && v01 == v01a);
        EXPECT_TRUE(v01.type() == value_type::none && v01.data().size() == 0);

        auto v02  = h.read_value(TEXT("val_02"));
        auto v02a = h.read_value(TEXT("val_02"), ec);
        EXPECT_TRUE(!ec && v02 == v02a);
        EXPECT_TRUE(v02.type() == value_type::sz && v02.to_string() == TEXT("42"));

        auto v03  = h.read_value(TEXT("val_03"));
        auto v03a = h.read_value(TEXT("val_03"), ec);
        EXPECT_TRUE(!ec && v03 == v03a);
        EXPECT_TRUE(v03.type() == value_type::expand_sz && v03.to_string() == TEXT("42"));

        auto v04  = h.read_value(TEXT("val_04"));
        auto v04a = h.read_value(TEXT("val_04"), ec);
        EXPECT_TRUE(!ec && v04 == v04a);
        EXPECT_TRUE(v04.type() == value_type::binary && (v04.to_byte_array() == byte_array_type{ 4, 2 }));

        auto v05  = h.read_value(TEXT("val_05"));
        auto v05a = h.read_value(TEXT("val_05"), ec);
        EXPECT_TRUE(!ec && v05 == v05a);
        EXPECT_TRUE(v05.type() == value_type::dword && v05.to_uint32() == 42);

        auto v06  = h.read_value(TEXT("val_06"));
        auto v06a = h.read_value(TEXT("val_06"), ec);
        EXPECT_TRUE(!ec && v06 == v06a);
        EXPECT_TRUE(v06.type() == value_type::dword_big_endian && v06.to_uint32() == 42);

        auto v07  = h.read_value(TEXT("val_07"));
        auto v07a = h.read_value(TEXT("val_07"), ec);
        EXPECT_TRUE(!ec && v07 == v07a);
        EXPECT_TRUE(v07.type() == value_type::link && v07.to_string() == TEXT("42"));

        auto v08  = h.read_value(TEXT("val_08"));
        auto v08a = h.read_value(TEXT("val_08"), ec);
        EXPECT_TRUE(!ec && v08 == v08a);
        EXPECT_TRUE(v08.type() == value_type::multi_sz && (v08.to_strings() == 
            std::vector<string_type>{ TEXT("42"), TEXT("42") }));

        auto v09  = h.read_value(TEXT("val_09"));
        auto v09a = h.read_value(TEXT("val_09"), ec);
        EXPECT_TRUE(!ec && v09 == v09a);
        EXPECT_TRUE(v09.type() == value_type::qword && v09.to_uint64() == 42);

        int exceptions = 0;
        try {
            h.read_value(TEXT("non_existent"));
        } catch(const registry_error& e) {
            ++exceptions;
            EXPECT_TRUE(e.path1() == h.path());
            EXPECT_TRUE(e.path2() == key_path());
            EXPECT_TRUE(e.value_name() == TEXT("non_existent"));
        }
        EXPECT_TRUE(exceptions == 1);

        auto v10 = h.read_value(TEXT("non_existent"), ec);
        EXPECT_TRUE(ec && v10 == value());
    }

    // key_handle::create_key(const key_path&, access_rights)
    // key_handle::create_key(const key_path&, access_rights, std::error_code&)
    {
        // create the parent key
        const key_path pk = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write");
        EXPECT_TRUE(!key_exists(pk) && create_key(pk) && key_exists(pk));

        std::error_code ec;
        const key_path sk1 = TEXT("new_key_1");
        const key_path sk2 = TEXT("new_key_2");
        const key_path sk3 = TEXT("new_key_3\\Inner1\\Inner2");
        const key_path sk4 = TEXT("new_key_4\\Inner1\\Inner2");
        const key_handle h(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write"), access_rights::create_sub_key);

        // create new keys (without subkeys)
        const key_path new_key1 = h.path().append(sk1);
        const key_path new_key2 = h.path().append(sk2);
        EXPECT_TRUE(!key_exists(new_key1) && !key_exists(new_key2));      // check that the keys does not exist
        auto ret1 = h.create_key(sk1, access_rights::all_access);         // create the first key
        EXPECT_TRUE(ret1.second == true && key_exists(new_key1));         // the key was created
        EXPECT_TRUE(ret1.first.is_open()          && 
                    ret1.first.path() == new_key1 && 
                    ret1.first.rights() == access_rights::all_access);    // and we have a valid result
        //
        auto ret2 = h.create_key(sk2, access_rights::all_access, ec);     // create the second key
        EXPECT_TRUE(!ec && ret2.second == true && key_exists(new_key2));  // the key was created
        EXPECT_TRUE(ret2.first.is_open()          && 
                    ret2.first.path() == new_key2 && 
                    ret2.first.rights() == access_rights::all_access);    // and we have a valid result

        // create new keys (with subkeys)
        const key_path new_key3 = h.path().append(sk3);
        const key_path new_key4 = h.path().append(sk4);
        EXPECT_TRUE(!key_exists(new_key3) && !key_exists(new_key4));      // check that the keys does not exist
        auto ret3 = h.create_key(sk3, access_rights::all_access);         // create the first key
        EXPECT_TRUE(ret3.second == true && key_exists(new_key3));         // the key was created
        EXPECT_TRUE(ret3.first.is_open()          &&
                    ret3.first.path() == new_key3 &&
                    ret3.first.rights() == access_rights::all_access);    // and we have a valid result
        auto ret4 = h.create_key(sk4, access_rights::all_access, ec);
        EXPECT_TRUE(!ec && ret4.second == true && key_exists(new_key4));  // the key was created
        EXPECT_TRUE(ret4.first.is_open()          &&
                    ret4.first.path() == new_key4 && 
                    ret4.first.rights() == access_rights::all_access); // and we have a valid result

        // obtain a new handle to to the same key
        auto ret5 = h.create_key(key_path(), access_rights::all_access);
        EXPECT_TRUE(ret5.second == false);                     // the key was not created
        EXPECT_TRUE(ret5.first.is_open() && ret5.first != h);  // we have a valid new handle ...
        EXPECT_TRUE(ret5.first.path() == h.path() && ret5.first.rights() == access_rights::all_access); // to the same key
        //
        auto ret6 = h.create_key(key_path(), access_rights::all_access, ec);
        EXPECT_TRUE(!ec && ret6.second == false);              // the key was not created
        EXPECT_TRUE(ret6.first.is_open() && ret6.first != h);  // we have a valid new handle ...
        EXPECT_TRUE(ret6.first.path() == h.path() && ret6.first.rights() == access_rights::all_access); // to the same key

        // try create already existing keys
        auto ret7 = h.create_key(sk1, access_rights::all_access);
        EXPECT_TRUE(ret7.second == false);                     // the key was not created
        EXPECT_TRUE(ret7.first.is_open() && ret7.first != h);  // we have a valid new handle ...
        EXPECT_TRUE(ret7.first.path() == new_key1 && ret7.first.rights() == access_rights::all_access); // to the same key
        //
        auto ret8 = h.create_key(sk1, access_rights::all_access, ec);
        EXPECT_TRUE(!ec && ret8.second == false);              // the key was not created
        EXPECT_TRUE(ret8.first.is_open() && ret8.first != h);  // we have a valid new handle ...
        EXPECT_TRUE(ret8.first.path() == new_key1 && ret8.first.rights() == access_rights::all_access); // to the same key
    }

    // key_handle::write_value(string_view_type, const value&)
    // key_handle::write_value(string_view_type, const value&, std::error_code&)
    {
        std::error_code ec;
        const uint8_t bytes[] = { 4, 2};
        const key_handle h(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write"), 
                           access_rights::set_value | access_rights::query_value);
        
        const value v01(none_value_tag{});
        h.write_value(TEXT("val_01"), v01);
        EXPECT_TRUE((h.write_value(TEXT("val_01a"), v01, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_01")) == v01 && h.read_value(TEXT("val_01a")) == v01);

        const value v02(sz_value_tag{}, TEXT("42"));
        h.write_value(TEXT("val_02"), v02);
        EXPECT_TRUE((h.write_value(TEXT("val_02a"), v02, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_02")) == v02 && h.read_value(TEXT("val_02a")) == v02);

        const value v03(expand_sz_value_tag{}, TEXT("42"));
        h.write_value(TEXT("val_03"), v03);
        EXPECT_TRUE((h.write_value(TEXT("val_03a"), v03, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_03")) == v03 && h.read_value(TEXT("val_03a")) == v03);

        const value v04(binary_value_tag{}, { bytes, sizeof(bytes) });
        h.write_value(TEXT("val_04"), v04);
        EXPECT_TRUE((h.write_value(TEXT("val_04a"), v04, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_04")) == v04 && h.read_value(TEXT("val_04a")) == v04);

        const value v05(dword_value_tag{}, 42);
        h.write_value(TEXT("val_05"), v05);
        EXPECT_TRUE((h.write_value(TEXT("val_05a"), v05, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_05")) == v05 && h.read_value(TEXT("val_05a")) == v05);

        const value v06(dword_big_endian_value_tag{}, 42);
        h.write_value(TEXT("val_06"), v06);
        EXPECT_TRUE((h.write_value(TEXT("val_06a"), v06, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_06")) == v06 && h.read_value(TEXT("val_06a")) == v06);

        const value v07(link_value_tag{}, TEXT("42"));
        h.write_value(TEXT("val_07"), v07);
        EXPECT_TRUE((h.write_value(TEXT("val_07a"), v07, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_07")) == v07 && h.read_value(TEXT("val_07a")) == v07);

        const value v08(multi_sz_value_tag{}, { TEXT("42"), TEXT("42") });
        h.write_value(TEXT("val_08"), v08);
        EXPECT_TRUE((h.write_value(TEXT("val_08a"), v08, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_08")) == v08 && h.read_value(TEXT("val_08a")) == v08);

        const value v09(qword_value_tag{}, 42);
        h.write_value(TEXT("val_09"), v09);
        EXPECT_TRUE((h.write_value(TEXT("val_09a"), v09, ec), !ec));
        EXPECT_TRUE(h.read_value(TEXT("val_09")) == v09 && h.read_value(TEXT("val_09a")) == v09);  
    }

    // key_handle::remove_key(string_view_type)
    // key_handle::remove_key(string_view_type, std::error_code&)
    {
        // TODO: ...

        //std::error_code ec;
        //const key_handle h(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1"), access_rights::set_value);

        //EXPECT_TRUE(!h.exists(TEXT("non_existing")));
        //EXPECT_TRUE(h.exists(TEXT("val_01")) && h.exists(TEXT("val_02")));

        // remove an non-existing value
        //EXPECT_TRUE(h.remove(TEXT("non_existing")) == false);
        //EXPECT_TRUE(h.remove(TEXT("non_existing"), ec) == false && !ec);

        // remove an existing value
        //EXPECT_TRUE(h.remove(TEXT("val_01")) == true && !h.exists(TEXT("val_01")));
        //EXPECT_TRUE(h.remove(TEXT("val_02"), ec) == true && !ec && !h.exists(TEXT("val_02")));
    }

    // key_handle::remove_keys(string_view_type)
    // key_handle::remove_keys(string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key_path p0 = TEXT("non_existing");
        const key_path p1 = TEXT("new_key_3");
        const key_path p2 = TEXT("new_key_4");
        const key_handle h(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write"), access_rights::query_value);

        EXPECT_TRUE(!key_exists(h.path().append(p0)));
        EXPECT_TRUE(key_exists(h.path().append(p1)) && info(h.path().append(p1)).subkeys > 0);
        EXPECT_TRUE(key_exists(h.path().append(p2)) && info(h.path().append(p2)).subkeys > 0);

        // remove an non-existing key
        EXPECT_TRUE(h.remove_keys(p0) == 0);
        EXPECT_TRUE(h.remove_keys(p0, ec) == 0 && !ec);

        // remove an non-empty key (which have subkeys)
        EXPECT_TRUE(h.remove_keys(p1) == 3 && !key_exists(h.path().append(p1)));
        EXPECT_TRUE(h.remove_keys(p2, ec) == 3 && !ec && !key_exists(h.path().append(p2)));

        // some clean-up
        remove_keys(h.path());
    }
}