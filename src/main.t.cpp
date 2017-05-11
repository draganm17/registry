#include <array>
#include <cstdint>
#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/endian/arithmetic.hpp>

#include <registry/registry.h>

using namespace registry;


void clear_stage() noexcept
{
    const LRESULT rc = RegDeleteTree(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry"));
    assert(rc == ERROR_SUCCESS || rc == ERROR_FILE_NOT_FOUND);
}

void prepare_stage() noexcept
{
    clear_stage();

    //create keys
    //
    HKEY hkey;
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read"), 
           &hkey) == ERROR_SUCCESS);               RegCloseKey(hkey);
    //
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_1_deep_0"), 
                        &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_1_deep_0\\key_1_deep_1"), 
                        &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_1_deep_0\\key_2_deep_1"), 
                        &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_1_deep_0\\key_2_deep_1\\key_1_deep_2"), 
                        &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_1_deep_0\\key_2_deep_1\\key_2_deep_2"), 
                        &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    //
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_2_deep_0"), 
                        &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    //
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_3_deep_0"), 
           &hkey) == ERROR_SUCCESS);               RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_3_deep_0\\key_1_deep_1"), 
                        &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read\\key_3_deep_0\\key_2_deep_1"), 
           &hkey) == ERROR_SUCCESS);               RegCloseKey(hkey);


    //create values
    //
    assert(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\read"), 
                        0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS);
    {
        assert(RegSetValueEx(hkey, TEXT("val_01"), 0, REG_NONE, nullptr, 0) == ERROR_SUCCESS);
    }
    {
        const string_type::value_type str[] = TEXT("42");
        assert(RegSetValueEx(hkey, TEXT("val_02"), 0, REG_SZ, 
                             reinterpret_cast<const uint8_t*>(str), sizeof(str)) == ERROR_SUCCESS);
    }
    {
        const string_type::value_type str[] = TEXT("42");
        assert(RegSetValueEx(hkey, TEXT("val_03"), 0, REG_EXPAND_SZ, 
                             reinterpret_cast<const uint8_t*>(str), sizeof(str)) == ERROR_SUCCESS);
    }
    {
        std::array<uint8_t, 2> data{ 4, 2 };
        assert(RegSetValueEx(hkey, TEXT("val_04"), 0, REG_BINARY, data.data(), (DWORD)data.size()) == ERROR_SUCCESS);
    }
    {
        const uint32_t val = 42;
        assert(RegSetValueEx(hkey, TEXT("val_05"), 0, REG_DWORD, 
                             reinterpret_cast<const uint8_t*>(&val), sizeof(val)) == ERROR_SUCCESS);
    }
    {
        const boost::endian::big_uint32_t val = 42;
        assert(RegSetValueEx(hkey, TEXT("val_06"), 0, REG_DWORD_BIG_ENDIAN, 
                             reinterpret_cast<const uint8_t*>(&val), sizeof(val)) == ERROR_SUCCESS);
    }
    {
        const string_type::value_type str[] = TEXT("42");
        assert(RegSetValueEx(hkey, TEXT("val_07"), 0, REG_LINK, 
                             reinterpret_cast<const uint8_t*>(str), sizeof(str)) == ERROR_SUCCESS);
    }
    {
        string_type::value_type strs[] = { TEXT('4'), TEXT('2'), TEXT('\0'),
                                           TEXT('4'),TEXT('2'),TEXT('\0'),TEXT('\0') };
        assert(RegSetValueEx(hkey, TEXT("val_08"), 0, REG_MULTI_SZ, 
                             reinterpret_cast<const uint8_t*>(strs), sizeof(strs)) == ERROR_SUCCESS);
    }
    {
        const uint64_t val = 42;
        assert(RegSetValueEx(hkey, TEXT("val_09"), 0, REG_QWORD, 
                             reinterpret_cast<const uint8_t*>(&val), sizeof(val)) == ERROR_SUCCESS);
    }
    RegCloseKey(hkey);
}


int main(int argc, char** argv)
{
    const key_path p = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry");

    auto rv1 = read_value(p, TEXT("test1"));
    auto rv2 = read_value(p, TEXT("test2"));
    auto rv3 = read_value(p, TEXT("test3"));
    auto rv4 = read_value(p, TEXT("test4"));
    //
    auto cvt1 = rv1.to_string();
    auto cvt2 = rv2.to_string();
    auto cvt3 = rv3.to_string();
    auto cvt4 = rv4.to_strings();

    prepare_stage();
    ::testing::InitGoogleMock(&argc, argv);


    value val1(sz_value_tag{},        TEXT("test"));
    value val2(expand_sz_value_tag{}, TEXT("test"));
    value val3(link_value_tag{},      TEXT("test"));
    value val4(multi_sz_value_tag{},  { TEXT("test") });

    write_value(p, TEXT("test1"), val1);
    write_value(p, TEXT("test2"), val2);
    write_value(p, TEXT("test3"), val3);
    write_value(p, TEXT("test4"), val4);


    const auto rc = RUN_ALL_TESTS();

    clear_stage();
    return rc;
}