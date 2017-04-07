#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/operations.h>

using namespace registry;


TEST(Operations, All)
{
    // space()
    // space(std::error_code&)
    {
        std::error_code ec;
        const space_info info1 = space();
        const space_info info2 = space(ec);

        EXPECT_TRUE(info1.capacity > 0);
        EXPECT_TRUE(info1.size > 0 && info1.size <= info1.capacity);

        EXPECT_TRUE(!ec && info2.capacity > 0);
        EXPECT_TRUE(!ec && info2.size > 0 && info2.size <= info2.capacity);
    }

    // exists(const key&)
    // exists(const key&, std::error_code&)
    {
        std::error_code ec;
        EXPECT_TRUE(exists(key::from_key_id(key_id::local_machine)) == true);
        EXPECT_TRUE(exists(key::from_key_id(key_id::local_machine), ec) == true && !ec);

        EXPECT_TRUE(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry")) == true);
        EXPECT_TRUE(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), ec) == true && !ec);

        EXPECT_TRUE(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\non_existent")) == false);
        EXPECT_TRUE(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\non_existent"), ec) == false && !ec);
    }

    // exists(const key&, string_view_type)
    // exists(const key&, string_view_type, std::error_code&)
    {
        std::error_code ec;
        EXPECT_TRUE(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), TEXT("val_01")) == true);
        EXPECT_TRUE(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), TEXT("val_01"), ec) == true && !ec);

        EXPECT_TRUE(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), TEXT("non_existent")) == false);
        EXPECT_TRUE(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), TEXT("non_existent"), ec) == false && !ec);
    }

    // info(const key&)
    // info(const key&, std::error_code&)
    {
        // TODO: ...
    }

    // read_value(const key&, string_view_type)
    // read_value(const key&, string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry");
            
        auto v01  = read_value(k, TEXT("val_01"));
        auto v01a = read_value(k, TEXT("val_01"), ec);
        EXPECT_TRUE(!ec && v01 == v01a);

        auto v02  = read_value(k, TEXT("val_02"));
        auto v02a = read_value(k, TEXT("val_02"), ec);
        EXPECT_TRUE(!ec && v02 == v02a);

        auto v03  = read_value(k, TEXT("val_03"));
        auto v03a = read_value(k, TEXT("val_03"), ec);
        EXPECT_TRUE(!ec && v03 == v03a);

        auto v04  = read_value(k, TEXT("val_04"));
        auto v04a = read_value(k, TEXT("val_04"), ec);
        EXPECT_TRUE(!ec && v04 == v04a);

        auto v05  = read_value(k, TEXT("val_05"));
        auto v05a = read_value(k, TEXT("val_05"), ec);
        EXPECT_TRUE(!ec && v05 == v05a);

        auto v06  = read_value(k, TEXT("val_06"));
        auto v06a = read_value(k, TEXT("val_06"), ec);
        EXPECT_TRUE(!ec && v06 == v06a);

        auto v07  = read_value(k, TEXT("val_07"));
        auto v07a = read_value(k, TEXT("val_07"), ec);
        EXPECT_TRUE(!ec && v07 == v07a);

        auto v08  = read_value(k, TEXT("val_08"));
        auto v08a = read_value(k, TEXT("val_08"), ec);
        EXPECT_TRUE(!ec && v08 == v08a);

        auto v09  = read_value(k, TEXT("val_09"));
        auto v09a = read_value(k, TEXT("val_09"), ec);
        EXPECT_TRUE(!ec && v09 == v09a);

        auto v10 = read_value(k, TEXT("non_existent"), ec);
        EXPECT_TRUE(ec && v10 == value());

        int exceptions = 0;
        try {
            read_value(k, TEXT("non_existent"));
        } catch(const registry_error& e) {
            ++exceptions;
            EXPECT_TRUE(e.key1() == k);
            EXPECT_TRUE(e.key2() == key());
            EXPECT_TRUE(e.value_name() == TEXT("non_existent"));
        }
        EXPECT_TRUE(exceptions == 1);
            
        EXPECT_TRUE(v01.type() == value_type::none             && v01.data().size() == 0);
        EXPECT_TRUE(v02.type() == value_type::sz               && v02.to_string() == TEXT("42"));
        EXPECT_TRUE(v03.type() == value_type::expand_sz        && v03.to_string() == TEXT("42"));
        EXPECT_TRUE(v04.type() == value_type::binary           && (v04.to_byte_array() == byte_array_type{ 4, 2 }));
        EXPECT_TRUE(v05.type() == value_type::dword            && v05.to_uint32() == 42);
        EXPECT_TRUE(v06.type() == value_type::dword_big_endian && v06.to_uint32() == 42);
        EXPECT_TRUE(v07.type() == value_type::link             && v07.to_string() == TEXT("42"));
        EXPECT_TRUE(v08.type() == value_type::multi_sz         && (v08.to_strings() == 
                                                                   std::vector<string_type>{ TEXT("42"), TEXT("42") }));
        EXPECT_TRUE(v09.type() == value_type::qword            && v09.to_uint64() == 42);
    }

    // create_key(const key& key)
    // create_key(const key& key, std::error_code&)
    {
        std::error_code ec;
        const key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1");
        const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_2");
        const key k3 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_3\\Inner1\\Inner2");
        const key k4 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_4\\Inner1\\Inner2");

        // create new keys (without subkeys)
        EXPECT_TRUE(!exists(k1) && !exists(k2));
        EXPECT_TRUE(create_key(k1) == true && exists(k1));
        EXPECT_TRUE(create_key(k2, ec) == true && !ec && exists(k2));

        // create new keys (with subkeys)
        EXPECT_TRUE(!exists(k3) && !exists(k4));
        EXPECT_TRUE(create_key(k3) == true && exists(k3));
        EXPECT_TRUE(create_key(k4, ec) == true && !ec && exists(k4));

        // try create already existing keys
        EXPECT_TRUE(create_key(k1) == false);
        EXPECT_TRUE(create_key(k2, ec) == false && !ec);
        EXPECT_TRUE(create_key(key::from_key_id(key_id::current_user)) == false);
        EXPECT_TRUE(create_key(key::from_key_id(key_id::current_user), ec) == false && !ec);
    }

    // write_value(const key&, string_view_type, const value&)
    // write_value(const key&, string_view_type, const value&, std::error_code&)
    {
        const uint8_t bytes[] = { 4, 2};
        const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1");
        
        const value v01(none_value_tag{});
        const value v02(sz_value_tag{},               TEXT("42"));
        const value v03(expand_sz_value_tag{},        TEXT("42"));
        const value v04(binary_value_tag{},           { bytes, sizeof(bytes) });
        const value v05(dword_value_tag{},            42);
        const value v06(dword_big_endian_value_tag{}, 42);
        const value v07(link_value_tag{},             TEXT("42"));
        const value v08(multi_sz_value_tag{},         { TEXT("42"), TEXT("42") });
        const value v09(qword_value_tag{},            42);

        write_value(k, TEXT("val_01"), v01);
        write_value(k, TEXT("val_02"), v02);
        write_value(k, TEXT("val_03"), v03);
        write_value(k, TEXT("val_04"), v04);
        write_value(k, TEXT("val_05"), v05);
        write_value(k, TEXT("val_06"), v06);
        write_value(k, TEXT("val_07"), v07);
        write_value(k, TEXT("val_08"), v08);
        write_value(k, TEXT("val_09"), v09);

        std::error_code ec;
        EXPECT_TRUE((write_value(k, TEXT("val_01a"), v01, ec), !ec));
        EXPECT_TRUE((write_value(k, TEXT("val_02a"), v02, ec), !ec));
        EXPECT_TRUE((write_value(k, TEXT("val_03a"), v03, ec), !ec));
        EXPECT_TRUE((write_value(k, TEXT("val_04a"), v04, ec), !ec));
        EXPECT_TRUE((write_value(k, TEXT("val_05a"), v05, ec), !ec));
        EXPECT_TRUE((write_value(k, TEXT("val_06a"), v06, ec), !ec));
        EXPECT_TRUE((write_value(k, TEXT("val_07a"), v07, ec), !ec));
        EXPECT_TRUE((write_value(k, TEXT("val_08a"), v08, ec), !ec));
        EXPECT_TRUE((write_value(k, TEXT("val_09a"), v09, ec), !ec));

        EXPECT_TRUE(read_value(k, TEXT("val_01")) == v01 && read_value(k, TEXT("val_01a")) == v01);
        EXPECT_TRUE(read_value(k, TEXT("val_02")) == v02 && read_value(k, TEXT("val_02a")) == v02);
        EXPECT_TRUE(read_value(k, TEXT("val_03")) == v03 && read_value(k, TEXT("val_03a")) == v03);
        EXPECT_TRUE(read_value(k, TEXT("val_04")) == v04 && read_value(k, TEXT("val_04a")) == v04);
        EXPECT_TRUE(read_value(k, TEXT("val_05")) == v05 && read_value(k, TEXT("val_05a")) == v05);
        EXPECT_TRUE(read_value(k, TEXT("val_06")) == v06 && read_value(k, TEXT("val_06a")) == v06);
        EXPECT_TRUE(read_value(k, TEXT("val_07")) == v07 && read_value(k, TEXT("val_07a")) == v07);
        EXPECT_TRUE(read_value(k, TEXT("val_08")) == v08 && read_value(k, TEXT("val_08a")) == v08);
        EXPECT_TRUE(read_value(k, TEXT("val_09")) == v09 && read_value(k, TEXT("val_09a")) == v09);
    }

    // remove(const key&, string_view_type)
    // remove(const key&, string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1");

        EXPECT_TRUE(!exists(k, TEXT("non_existing")));
        EXPECT_TRUE(exists(k, TEXT("val_01")) && exists(k, TEXT("val_02")));

        // remove an non-existing value
        EXPECT_TRUE(remove(k, TEXT("non_existing")) == false);
        EXPECT_TRUE(remove(k, TEXT("non_existing"), ec) == false && !ec);

        // remove an existing value
        EXPECT_TRUE(remove(k, TEXT("val_01")) == true && !exists(k, TEXT("val_01")));
        EXPECT_TRUE(remove(k, TEXT("val_02"), ec) == true && !ec && !exists(k, TEXT("val_02")));
    }

    // remove(const key&)
    // remove(const key&, std::error_code&)
    {
        std::error_code ec;
        const key k0 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existing");
        const key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1");
        const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_2");
        const key k3 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_3");

        EXPECT_TRUE(!exists(k0) && exists(k1) && exists(k2));
        EXPECT_TRUE(exists(key(k3).append(TEXT("Inner1\\Inner2"))));

        // remove an non-existing key
        EXPECT_TRUE(remove(k0) == false);
        EXPECT_TRUE(remove(k0, ec) == false && !ec);

        // remove an empty key (with no subkeys)
        EXPECT_TRUE(remove(k1) == true && !exists(k1));
        EXPECT_TRUE(remove(k2, ec) == true && !ec && !exists(k2));

        // try remove an non-empty key (which have subkeys)
        int exception = 0;
        EXPECT_TRUE(remove(k3, ec) == false && ec && exists(k3));
        try {
            remove(k3);
        } catch (const registry_error& e) {
            ++exception;
            exists(k3);
        }
        EXPECT_TRUE(exception == 1);
    }

    // remove_all(const key&)
    // remove_all(const key&, std::error_code&)
    {
        // TODO: ...

        //std::error_code ec;
        //const key k0 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existing");
        //const key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_3");
        //const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_4");

        //EXPECT_TRUE(!exists(k0));
        //EXPECT_TRUE(exists(key(k1).append(TEXT("Inner1\\Inner2"))));
        //EXPECT_TRUE(exists(key(k2).append(TEXT("Inner1\\Inner2"))));

        // remove an non-existing key
        //EXPECT_TRUE(remove_all(k0) == 0);
        //EXPECT_TRUE(remove_all(k0, ec) == 0 && !ec);

        // remove an non-empty key (which have subkeys)
        //EXPECT_TRUE(remove_all(k1) == 3 && !exists(k1));
        //EXPECT_TRUE(remove_all(k2, ec) == 3 && !ec && !exists(k2));
    }


    //equivalent
    {
#if defined(_WIN_64)
        const bool is_64bit_machine = true;
#else
        BOOL f64 = FALSE;
        const bool is_64bit_machine = IsWow64Process(GetCurrentProcess(), &f64) && f64;
#endif

        const key k1(TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE"), view::view_32bit);
        const key k2(TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE"), view::view_64bit);

        auto ans = equivalent(k1, k2);
        EXPECT_TRUE((is_64bit_machine && ans == false) || (!is_64bit_machine && ans == true));
    }
}