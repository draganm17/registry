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
        EXPECT_TRUE(!h.valid());
    }

    // key_handle::key_handle(const weak_key_handle&);
    {
        // TODO: ...
    }
}

TEST(KeyHandle, Factories)
{
    std::error_code ec;
    const key k1 = key::from_key_id(key_id::current_user);
    const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry");
    const key k3 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\non_existent");

    // open a predefined key
    const auto h1 = open(k1, access_rights::query_value);
    EXPECT_TRUE(h1.valid());
    EXPECT_TRUE(h1.key() == k1);
    EXPECT_TRUE(h1.rights() == access_rights::query_value);
    EXPECT_TRUE(h1.native_handle() != key_handle::native_handle_type{});

    const auto h2 = open(k1, access_rights::query_value, ec);
    EXPECT_TRUE(!ec);
    EXPECT_TRUE(h2.valid());
    EXPECT_TRUE(h2.key() == k1);
    EXPECT_TRUE(h2.rights() == access_rights::query_value);
    EXPECT_TRUE(h2.native_handle() != key_handle::native_handle_type{});

    // open a regular key
    const auto h3 = open(k2, access_rights::query_value);
    EXPECT_TRUE(h3.valid());
    EXPECT_TRUE(h3.key() == k2);
    EXPECT_TRUE(h3.rights() == access_rights::query_value);
    EXPECT_TRUE(h3.native_handle() != key_handle::native_handle_type{});

    const auto h4 = open(k2, access_rights::query_value, ec);
    EXPECT_TRUE(!ec);
    EXPECT_TRUE(h4.valid());
    EXPECT_TRUE(h4.key() == k2);
    EXPECT_TRUE(h4.rights() == access_rights::query_value);
    EXPECT_TRUE(h4.native_handle() != key_handle::native_handle_type{});

    // try open an non-existing key
    int exceptions = 0;
    try {
        const auto h5 = open(k3, access_rights::query_value);
    } catch (const registry_error& e) {
        ++exceptions;
    }
    EXPECT_TRUE(exceptions == 1);

    const auto h6 = open(k3, access_rights::query_value, ec);
    EXPECT_TRUE(ec && !h6.valid());
}

TEST(KeyHandle, GettersAndQueries)
{
    static const auto test = [](key_id id)
    {
        const key_handle h = id;
        if (id != key_id::unknown) {
            EXPECT_TRUE(h.valid());
            EXPECT_TRUE(h.key() == key::from_key_id(id));
            EXPECT_TRUE(h.rights() == access_rights::unknown);
            EXPECT_TRUE(h.native_handle() == static_cast<key_handle::native_handle_type>(id));
        } else {
            EXPECT_TRUE(!h.valid());
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
    // key_handle::exists(string_view_type)
    // key_handle::exists(string_view_type, std::error_code&)
    {
        std::error_code ec;
        const auto h = open(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), access_rights::query_value);

        EXPECT_TRUE(h.exists(TEXT("val_01")) == true);
        EXPECT_TRUE(h.exists(TEXT("val_01"), ec) == true && !ec);

        EXPECT_TRUE(h.exists(TEXT("non_existent")) == false);
        EXPECT_TRUE(h.exists(TEXT("non_existent"), ec) == false && !ec);
    }

    // key_handle::info()
    // key_handle::info(std::error_code&)
    {
        // TODO: ...
    }

    // key_handle::read_value(string_view_type)
    // key_handle::read_value(string_view_type, std::error_code&)
    {
        std::error_code ec;
        auto h = open(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), access_rights::query_value);
            
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
            EXPECT_TRUE(e.key1() == h.key());
            EXPECT_TRUE(e.key2() == key());
            EXPECT_TRUE(e.value_name() == TEXT("non_existent"));
        }
        EXPECT_TRUE(exceptions == 1);

        auto v10 = h.read_value(TEXT("non_existent"), ec);
        EXPECT_TRUE(ec && v10 == value());
    }

    // key_handle::create_key(const key& key, access_rights)
    // key_handle::create_key(const key& key, access_rights, std::error_code&)
    {
        // create the parent key
        const key pk = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write");
        EXPECT_TRUE(!exists(pk) && create_key(pk) && exists(pk));

        std::error_code ec;
        const key sk1 = TEXT("new_key_1");
        const key sk2 = TEXT("new_key_2");
        const key sk3 = TEXT("new_key_3\\Inner1\\Inner2");
        const key sk4 = TEXT("new_key_4\\Inner1\\Inner2");
        const auto h = open(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write"), access_rights::create_sub_key);

        // create new keys (without subkeys)
        const key new_key1 = h.key().append(sk1);
        const key new_key2 = h.key().append(sk2);
        EXPECT_TRUE(!exists(new_key1) && !exists(new_key2));           // check that the keys does not exist
        auto ret1 = h.create_key(sk1, access_rights::all_access);      // create the first key
        EXPECT_TRUE(ret1.second == true && exists(new_key1));          // the key was created
        EXPECT_TRUE(ret1.first.valid()           && 
                    ret1.first.key() == new_key1 && 
                    ret1.first.rights() == access_rights::all_access); // and we have a valid result
        //
        auto ret2 = h.create_key(sk2, access_rights::all_access, ec);  // create the second key
        EXPECT_TRUE(!ec && ret2.second == true && exists(new_key2));   // the key was created
        EXPECT_TRUE(ret2.first.valid()           && 
                    ret2.first.key() == new_key2 && 
                    ret2.first.rights() == access_rights::all_access); // and we have a valid result

        // create new keys (with subkeys)
        const key new_key3 = h.key().append(sk3);
        const key new_key4 = h.key().append(sk4);
        EXPECT_TRUE(!exists(new_key3) && !exists(new_key4));           // check that the keys does not exist
        auto ret3 = h.create_key(sk3, access_rights::all_access);      // create the first key
        EXPECT_TRUE(ret3.second == true && exists(new_key3));          // the key was created
        EXPECT_TRUE(ret3.first.valid()           && 
                    ret3.first.key() == new_key3 && 
                    ret3.first.rights() == access_rights::all_access); // and we have a valid result
        auto ret4 = h.create_key(sk4, access_rights::all_access, ec);
        EXPECT_TRUE(!ec && ret4.second == true && exists(new_key4));   // the key was created
        EXPECT_TRUE(ret4.first.valid()           && 
                    ret4.first.key() == new_key4 && 
                    ret4.first.rights() == access_rights::all_access); // and we have a valid result

        // obtain a new handle to to the same key
        auto ret5 = h.create_key(key(), access_rights::all_access);
        EXPECT_TRUE(ret5.second == false);                   // the key was not created
        EXPECT_TRUE(ret5.first.valid() && ret5.first != h);  // we have a valid new handle ...
        EXPECT_TRUE(ret5.first.key() == h.key() && ret5.first.rights() == access_rights::all_access); // to the same key
        //
        auto ret6 = h.create_key(key(), access_rights::all_access, ec);
        EXPECT_TRUE(!ec && ret6.second == false);            // the key was not created
        EXPECT_TRUE(ret6.first.valid() && ret6.first != h);  // we have a valid new handle ...
        EXPECT_TRUE(ret6.first.key() == h.key() && ret6.first.rights() == access_rights::all_access); // to the same key

        // try create already existing keys
        auto ret7 = h.create_key(sk1, access_rights::all_access);
        EXPECT_TRUE(ret7.second == false);                   // the key was not created
        EXPECT_TRUE(ret7.first.valid() && ret7.first != h);  // we have a valid new handle ...
        EXPECT_TRUE(ret7.first.key() == new_key1 && ret7.first.rights() == access_rights::all_access); // to the same key
        //
        auto ret8 = h.create_key(sk1, access_rights::all_access, ec);
        EXPECT_TRUE(!ec && ret8.second == false);            // the key was not created
        EXPECT_TRUE(ret8.first.valid() && ret8.first != h);  // we have a valid new handle ...
        EXPECT_TRUE(ret8.first.key() == new_key1 && ret8.first.rights() == access_rights::all_access); // to the same key
    }

    // key_handle::write_value(string_view_type, const value&)
    // key_handle::write_value(string_view_type, const value&, std::error_code&)
    {
        std::error_code ec;
        const uint8_t bytes[] = { 4, 2};
        const auto h = open(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write"), 
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

    // key_handle::remove(string_view_type)
    // key_handle::remove(string_view_type, std::error_code&)
    {
        // TODO: ...

        //std::error_code ec;
        //const auto h = open(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1"), access_rights::set_value);

        //EXPECT_TRUE(!h.exists(TEXT("non_existing")));
        //EXPECT_TRUE(h.exists(TEXT("val_01")) && h.exists(TEXT("val_02")));

        // remove an non-existing value
        //EXPECT_TRUE(h.remove(TEXT("non_existing")) == false);
        //EXPECT_TRUE(h.remove(TEXT("non_existing"), ec) == false && !ec);

        // remove an existing value
        //EXPECT_TRUE(h.remove(TEXT("val_01")) == true && !h.exists(TEXT("val_01")));
        //EXPECT_TRUE(h.remove(TEXT("val_02"), ec) == true && !ec && !h.exists(TEXT("val_02")));
    }

    // key_handle::remove_all(string_view_type)
    // key_handle::remove_all(string_view_type, std::error_code&)
    {
        // TODO: ...

        const LRESULT rc = RegDeleteTree(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\write"));
        assert(rc == ERROR_SUCCESS || rc == ERROR_FILE_NOT_FOUND);
    }
}