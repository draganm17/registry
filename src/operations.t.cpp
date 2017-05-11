#include <array>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/exception.h>
#include <registry/key_path.h>
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

    // key_exists(const key_path&)
    // key_exists(const key_path&, std::error_code&)
    {
        std::error_code ec;
        EXPECT_TRUE(key_exists(key_path::from_key_id(key_id::local_machine)) == true);
        EXPECT_TRUE(key_exists(key_path::from_key_id(key_id::local_machine), ec) == true && !ec);

        EXPECT_TRUE(key_exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry")) == true);
        EXPECT_TRUE(key_exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), ec) == true && !ec);

        EXPECT_TRUE(key_exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\non_existent")) == false);
        EXPECT_TRUE(key_exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\non_existent"), ec) == false && !ec);
    }

    // value_exists(const key_path&, string_view_type)
    // value_exists(const key_path&, string_view_type, std::error_code&)
    {
        std::error_code ec;
        EXPECT_TRUE(value_exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), TEXT("val_01")) == true);
        EXPECT_TRUE(value_exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), TEXT("val_01"), ec) == true && !ec);

        EXPECT_TRUE(value_exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), TEXT("non_existent")) == false);
        EXPECT_TRUE(value_exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read"), TEXT("non_existent"), ec) == false && !ec);
    }

    // info(const key_path&)
    // info(const key_path&, std::error_code&)
    {
        static const auto test_mask = [](const key_path& p, key_info_mask mask)
        {
            std::error_code ec;
            const key_info i1 = info(p, mask), i2 = info(p, mask, ec);
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
        const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");

        test_mask(p, key_info_mask::none);
        test_mask(p, key_info_mask::read_subkeys);
        test_mask(p, key_info_mask::read_values);
        test_mask(p, key_info_mask::read_max_subkey_size);
        test_mask(p, key_info_mask::read_max_value_name_size);
        test_mask(p, key_info_mask::read_max_value_data_size);
        test_mask(p, key_info_mask::read_last_write_time);
        test_mask(p, key_info_mask::all);
    }

    // read_value(const key_path&, string_view_type)
    // read_value(const key_path&, string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\read");
            
        auto v01  = read_value(p, TEXT("val_01"));
        auto v01a = read_value(p, TEXT("val_01"), ec);
        EXPECT_TRUE(!ec && v01 == v01a);
        EXPECT_TRUE(v01.type() == value_type::none && v01.data().size() == 0);

        auto v02  = read_value(p, TEXT("val_02"));
        auto v02a = read_value(p, TEXT("val_02"), ec);
        EXPECT_TRUE(!ec && v02 == v02a);
        EXPECT_TRUE(v02.type() == value_type::sz && v02.to_string() == TEXT("42"));

        auto v03  = read_value(p, TEXT("val_03"));
        auto v03a = read_value(p, TEXT("val_03"), ec);
        EXPECT_TRUE(!ec && v03 == v03a);
        EXPECT_TRUE(v03.type() == value_type::expand_sz && v03.to_string() == TEXT("42"));

        auto v04  = read_value(p, TEXT("val_04"));
        auto v04a = read_value(p, TEXT("val_04"), ec);
        EXPECT_TRUE(!ec && v04 == v04a);
        EXPECT_TRUE(v04.type() == value_type::binary && (v04.to_bytes() == byte_array_type{ 4, 2 }));

        auto v05  = read_value(p, TEXT("val_05"));
        auto v05a = read_value(p, TEXT("val_05"), ec);
        EXPECT_TRUE(!ec && v05 == v05a);
        EXPECT_TRUE(v05.type() == value_type::dword && v05.to_uint32() == 42);

        auto v06  = read_value(p, TEXT("val_06"));
        auto v06a = read_value(p, TEXT("val_06"), ec);
        EXPECT_TRUE(!ec && v06 == v06a);
        EXPECT_TRUE(v06.type() == value_type::dword_big_endian && v06.to_uint32() == 42);

        auto v07  = read_value(p, TEXT("val_07"));
        auto v07a = read_value(p, TEXT("val_07"), ec);
        EXPECT_TRUE(!ec && v07 == v07a);
        EXPECT_TRUE(v07.type() == value_type::link && v07.to_string() == TEXT("42"));

        auto v08  = read_value(p, TEXT("val_08"));
        auto v08a = read_value(p, TEXT("val_08"), ec);
        EXPECT_TRUE(!ec && v08 == v08a);
        EXPECT_TRUE(v08.type() == value_type::multi_sz && (v08.to_strings() ==
            std::vector<string_type>{ TEXT("42"), TEXT("42") }));

        auto v09  = read_value(p, TEXT("val_09"));
        auto v09a = read_value(p, TEXT("val_09"), ec);
        EXPECT_TRUE(!ec && v09 == v09a);
        EXPECT_TRUE(v09.type() == value_type::qword && v09.to_uint64() == 42);

        EXPECT_THROW(read_value(p, TEXT("non_existent")), registry_error);
        //
        auto v10 = read_value(p, TEXT("non_existent"), ec);
        EXPECT_TRUE(ec && v10 == value());  
    }

    // create_key(const key_path&)
    // create_key(const key_path&, std::error_code&)
    {
        std::error_code ec;
        const key_path p0 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write");
        const key_path p1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\new_key_1");
        const key_path p2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\new_key_2");
        const key_path p3 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\new_key_3\\Inner1\\Inner2");
        const key_path p4 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\new_key_4\\Inner1\\Inner2");

        // create the parent key
        ASSERT_TRUE(!key_exists(p0));
        EXPECT_TRUE(create_key(p0) && key_exists(p0));

        // create new keys (without subkeys)
        ASSERT_TRUE(!key_exists(p1) && !key_exists(p2));
        EXPECT_TRUE(create_key(p1) == true && key_exists(p1));
        EXPECT_TRUE(create_key(p2, ec) == true && !ec && key_exists(p2));

        // create new keys (with subkeys)
        ASSERT_TRUE(!key_exists(p3) && !key_exists(p4));
        EXPECT_TRUE(create_key(p3) == true && key_exists(p3));
        EXPECT_TRUE(create_key(p4, ec) == true && !ec && key_exists(p4));

        // try create already existing keys
        EXPECT_TRUE(create_key(p1) == false);
        EXPECT_TRUE(create_key(p2, ec) == false && !ec);
        EXPECT_TRUE(create_key(key_path::from_key_id(key_id::current_user)) == false);
        EXPECT_TRUE(create_key(key_path::from_key_id(key_id::current_user), ec) == false && !ec);
    }

    // write_value(const key_path&, string_view_type, const value&)
    // write_value(const key_path&, string_view_type, const value&, std::error_code&)
    {
        std::error_code ec;
        const std::array<uint8_t, 2> bytes{ 4, 2};
        const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write");
        
        const value v01(none_value_tag{});
        write_value(p, TEXT("val_01"), v01);
        EXPECT_TRUE((write_value(p, TEXT("val_01a"), v01, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_01")) == v01 && read_value(p, TEXT("val_01a")) == v01);

        const value v02(sz_value_tag{}, TEXT("42"));
        write_value(p, TEXT("val_02"), v02);
        EXPECT_TRUE((write_value(p, TEXT("val_02a"), v02, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_02")) == v02 && read_value(p, TEXT("val_02a")) == v02);

        const value v03(expand_sz_value_tag{}, TEXT("42"));
        write_value(p, TEXT("val_03"), v03);
        EXPECT_TRUE((write_value(p, TEXT("val_03a"), v03, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_03")) == v03 && read_value(p, TEXT("val_03a")) == v03);

        const value v04(binary_value_tag{}, bytes.data(), bytes.size());
        write_value(p, TEXT("val_04"), v04);
        EXPECT_TRUE((write_value(p, TEXT("val_04a"), v04, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_04")) == v04 && read_value(p, TEXT("val_04a")) == v04);

        const value v05(dword_value_tag{}, 42);
        write_value(p, TEXT("val_05"), v05);
        EXPECT_TRUE((write_value(p, TEXT("val_05a"), v05, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_05")) == v05 && read_value(p, TEXT("val_05a")) == v05);

        const value v06(dword_big_endian_value_tag{}, 42);
        write_value(p, TEXT("val_06"), v06);
        EXPECT_TRUE((write_value(p, TEXT("val_06a"), v06, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_06")) == v06 && read_value(p, TEXT("val_06a")) == v06);

        const value v07(link_value_tag{}, TEXT("42"));
        write_value(p, TEXT("val_07"), v07);
        EXPECT_TRUE((write_value(p, TEXT("val_07a"), v07, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_07")) == v07 && read_value(p, TEXT("val_07a")) == v07);

        const value v08(multi_sz_value_tag{}, { TEXT("42"), TEXT("42") });
        write_value(p, TEXT("val_08"), v08);
        EXPECT_TRUE((write_value(p, TEXT("val_08a"), v08, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_08")) == v08 && read_value(p, TEXT("val_08a")) == v08);

        const value v09(qword_value_tag{}, 42);
        write_value(p, TEXT("val_09"), v09);
        EXPECT_TRUE((write_value(p, TEXT("val_09a"), v09, ec), !ec));
        EXPECT_TRUE(read_value(p, TEXT("val_09")) == v09 && read_value(p, TEXT("val_09a")) == v09);  
    }

    // remove_value(const key_path&, string_view_type)
    // remove_value(const key_path&, string_view_type, std::error_code&)
    {
        std::error_code ec;
        const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write");

        ASSERT_TRUE(!value_exists(p, TEXT("non_existing")) &&
                     value_exists(p, TEXT("val_01"))       && 
                     value_exists(p, TEXT("val_02")));

        // remove an non-existing value
        EXPECT_TRUE(remove_value(p, TEXT("non_existing")) == false);
        EXPECT_TRUE(remove_value(p, TEXT("non_existing"), ec) == false && !ec);

        // remove an existing value
        EXPECT_TRUE(remove_value(p, TEXT("val_01")) == true && !value_exists(p, TEXT("val_01")));
        EXPECT_TRUE(remove_value(p, TEXT("val_02"), ec) == true && !ec && !value_exists(p, TEXT("val_02")));
    }

    // remove_key(const key_path&)
    // remove_key(const key_path&, std::error_code&)
    {
        std::error_code ec;
        
        // remove an non-existing key
        const key_path p1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\non_existing");
        ASSERT_TRUE(!key_exists(p1));
        EXPECT_TRUE(remove_key(p1) == false);
        EXPECT_TRUE(remove_key(p1, ec) == false && !ec);

        // remove an empty key (with no subkeys)
        const key_path p2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\new_key_1");
        ASSERT_TRUE(key_exists(p2) && info(p2).subkeys == 0);
        EXPECT_TRUE(remove_key(p2) == true && !key_exists(p2));
        //
        const key_path p3 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\new_key_2");
        ASSERT_TRUE(key_exists(p3) && info(p3).subkeys == 0);
        EXPECT_TRUE(remove_key(p3, ec) == true && !ec && !key_exists(p3));
    }

    // remove_keys(const key_path&)
    // remove_keys(const key_path&, std::error_code&)
    {
        std::error_code ec;
        const key_path p0 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existing");
        const key_path p1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\new_key_3");
        const key_path p2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\write\\new_key_4");

        EXPECT_TRUE(!key_exists(p0));
        EXPECT_TRUE(key_exists(p1) && info(p1).subkeys > 0);
        EXPECT_TRUE(key_exists(p2) && info(p2).subkeys > 0);

        // remove an non-existing key
        EXPECT_TRUE(remove_keys(p0) == 0);
        EXPECT_TRUE(remove_keys(p0, ec) == 0 && !ec);

        // remove an non-empty key (which have subkeys)
        EXPECT_TRUE(remove_keys(p1) == 3 && !key_exists(p1));
        EXPECT_TRUE(remove_keys(p2, ec) == 3 && !ec && !key_exists(p2));

        // some clean-up
        remove_keys(key_path(p1).remove_leaf_path());
    }

    //equivalent
    {
#if defined(_WIN_64)
        const bool is_64bit_machine = true;
#else
        BOOL f64 = FALSE;
        const bool is_64bit_machine = IsWow64Process(GetCurrentProcess(), &f64) && f64;
#endif

        const key_path p1(TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE"), view::view_32bit);
        const key_path p2(TEXT("HKEY_LOCAL_MACHINE\\SOFTWARE"), view::view_64bit);

        auto ans = equivalent(p1, p2);
        EXPECT_TRUE((is_64bit_machine && ans == false) || (!is_64bit_machine && ans == true));
    }
}