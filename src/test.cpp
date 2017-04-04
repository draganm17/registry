#include <array>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <Windows.h>

#include <boost/endian/arithmetic.hpp>
#include <boost/scope_exit.hpp>


#include <registry/registry.h>

void clear_stage() noexcept
{
    using namespace registry;
    const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry");

    remove_all(k);
    assert(exists(k) == false);
}

void prepare_stage() noexcept
{
    using namespace registry;

    clear_stage();

    //create keys
    //
    HKEY hkey;
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry"), &hkey) == ERROR_SUCCESS);                                            RegCloseKey(hkey);
    //
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_1_deep_0"), &hkey) == ERROR_SUCCESS);                              RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_1_deep_0\\key_1_deep_1"), &hkey) == ERROR_SUCCESS);                RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_1_deep_0\\key_2_deep_1"), &hkey) == ERROR_SUCCESS);                RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_1_deep_0\\key_2_deep_1\\key_1_deep_2"), &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_1_deep_0\\key_2_deep_1\\key_2_deep_2"), &hkey) == ERROR_SUCCESS);  RegCloseKey(hkey);
    //
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_2_deep_0"), &hkey) == ERROR_SUCCESS);                              RegCloseKey(hkey);
    //
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_3_deep_0"), &hkey) == ERROR_SUCCESS);                              RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_3_deep_0\\key_1_deep_1"), &hkey) == ERROR_SUCCESS);                RegCloseKey(hkey);
    assert(RegCreateKey(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry\\key_3_deep_0\\key_2_deep_1"), &hkey) == ERROR_SUCCESS);                RegCloseKey(hkey);


    //create values
    //
    assert(RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\libregistry"), 0, KEY_SET_VALUE, &hkey) == ERROR_SUCCESS);
    {
        assert(RegSetValueEx(hkey, TEXT("val_01"), 0, REG_NONE, nullptr, 0) == ERROR_SUCCESS);
    }
    {
        const string_type::value_type str[] = TEXT("42");
        assert(RegSetValueEx(hkey, TEXT("val_02"), 0, REG_SZ, reinterpret_cast<const uint8_t*>(str), sizeof(str)) == ERROR_SUCCESS);
    }
    {
        const string_type::value_type str[] = TEXT("42");
        assert(RegSetValueEx(hkey, TEXT("val_03"), 0, REG_EXPAND_SZ, reinterpret_cast<const uint8_t*>(str), sizeof(str)) == ERROR_SUCCESS);
    }
    {
        std::array<uint8_t, 2> data{ 4, 2 };
        assert(RegSetValueEx(hkey, TEXT("val_04"), 0, REG_BINARY, data.data(), (DWORD)data.size()) == ERROR_SUCCESS);
    }
    {
        const uint32_t val = 42;
        assert(RegSetValueEx(hkey, TEXT("val_05"), 0, REG_DWORD, reinterpret_cast<const uint8_t*>(&val), sizeof(val)) == ERROR_SUCCESS);
    }
    {
        const boost::endian::big_uint32_t val = 42;
        assert(RegSetValueEx(hkey, TEXT("val_06"), 0, REG_DWORD_BIG_ENDIAN, reinterpret_cast<const uint8_t*>(&val), sizeof(val)) == ERROR_SUCCESS);
    }
    {
        const string_type::value_type str[] = TEXT("42");
        assert(RegSetValueEx(hkey, TEXT("val_07"), 0, REG_LINK, reinterpret_cast<const uint8_t*>(str), sizeof(str)) == ERROR_SUCCESS);
    }
    {
        string_type::value_type strs[] = { TEXT('4'), TEXT('2'), TEXT('\0'), TEXT('4'),TEXT('2'),TEXT('\0'),TEXT('\0'), };
        assert(RegSetValueEx(hkey, TEXT("val_08"), 0, REG_MULTI_SZ, reinterpret_cast<const uint8_t*>(strs), sizeof(strs)) == ERROR_SUCCESS);
    }
    {
        const uint64_t val = 42;
        assert(RegSetValueEx(hkey, TEXT("val_09"), 0, REG_QWORD, reinterpret_cast<const uint8_t*>(&val), sizeof(val)) == ERROR_SUCCESS);
    }
    RegCloseKey(hkey);
}

struct TEST_REGISTRY_ERROR
{
    void operator()() noexcept
    {
        using namespace registry;

        // registry_error::registry_error(std::error_code, const std::string&)
        {
            const auto ec = std::make_error_code(std::errc::not_enough_memory);
            registry_error ex(ec, "test");

            assert(ex.key1() == key());
            assert(ex.key2() == key());
            assert(ex.value_name().empty());
        }

        // registry_error::registry_error(std::error_code, const std::string&, const key&)
        {
            const auto ec = std::make_error_code(std::errc::not_enough_memory);
            registry_error ex(ec, "test", key::from_key_id(key_id::current_user));

            assert(ex.key1() == key::from_key_id(key_id::current_user));
            assert(ex.key2() == key());
            assert(ex.value_name().empty());
        }

        // registry_error::registry_error(std::error_code, const std::string&, const key&, const key&)
        {
            const auto ec = std::make_error_code(std::errc::not_enough_memory);
            registry_error ex(ec, "test", key::from_key_id(key_id::current_user), key::from_key_id(key_id::local_machine));

            assert(ex.key1() == key::from_key_id(key_id::current_user));
            assert(ex.key2() == key::from_key_id(key_id::local_machine));
            assert(ex.value_name().empty());
        }

        // registry_error::registry_error(std::error_code, const std::string&, 
        //                                const key&, const key&, string_view_type)
        {
            const auto ec = std::make_error_code(std::errc::not_enough_memory);
            registry_error ex(ec, "test", key::from_key_id(key_id::current_user), 
                              key::from_key_id(key_id::local_machine), TEXT("test"));

            assert(ex.key1() == key::from_key_id(key_id::current_user));
            assert(ex.key2() == key::from_key_id(key_id::local_machine));
            assert(ex.value_name() == TEXT("test"));
        }
    }
};

struct TEST_KEY
{
    void operator()() noexcept
    {
        using namespace registry;

        // default constructor
        {
            key k;
            assert(k == key(string_type()));
        }

        // key::key(string_view_type, view)
        {
            key k(TEXT("HKEY_CURRENT_user\\Test"), view::view_32bit);
            assert(k.name() == TEXT("HKEY_CURRENT_user\\Test"));
            assert(k.view() == view::view_32bit);
        }

        // ==, != operators
        //{
        //    key k1, k2;
        //    key k3 = key_id::current_user;
        //    key k4 = key_id::current_user;
        //    key k5  (key_id::current_user, view::view_64bit);
        //    key k6 = TEXT("HKEY_LOCAL_MACHINE");
        //    key k7 = TEXT("HKEY_LOCAL_machine");
        //
        //    // a key is equal to itself
        //    assert(  k1 == k1 );  // testing operator ==
        //    assert(  k3 == k3 );
        //    assert(!(k1 != k1));  // testing operator !=
        //    assert(!(k3 != k3));
        //
        //    // two empty keys are equal
        //    assert(  k1 == k2 );  // testing operator ==
        //    assert(!(k1 != k2));  // testing operator !=
        //
        //    // an empty key is not equal to a non-empty one
        //    assert(!(k1 == k3));  // testing operator ==
        //    assert(  k1 != k3 );  // testing operator !=
        //
        //    // two non-empty identical keys are equal
        //    assert(  k3 == k4 );  // testing operator ==
        //    assert(!(k3 != k4));  // testing operator !=
        //
        //    // two different keys are not equal
        //    assert(!(k4 == k5));  // testing operator ==
        //    assert(!(k6 == k7));
        //    assert(  k4 != k5 );  // testing operator !=
        //    assert(  k6 != k7 );
        //}

        // key::compare(const key&)
        {
            assert(key().compare(key()) == 0);

            assert(key(TEXT("AAA")).compare(key(TEXT("AAA"))) == 0);

            assert(key(TEXT("AAA\\BBB")).compare(key(TEXT("AAA\\BBB"))) == 0);

            assert(key(TEXT("AAA\\BBB")).compare(key(TEXT("aAa\\\\bBb\\"))) == 0);

            assert(key(TEXT("AAA\\BBB")).compare(key(TEXT("AAA\\CCC"))) < 0);

            assert(key(TEXT("AAA\\BBB")).compare(key(TEXT("AAA\\AAA"))) > 0);

            assert(key(TEXT("AAA\\AAA"), view::view_32bit).compare(key(TEXT("AAA\\BBB"), view::view_64bit)) > 0);

            assert(key(TEXT("AAA\\BBB"), view::view_64bit).compare(key(TEXT("AAA\\AAA"), view::view_32bit)) < 0);
        }

        // key::begin()
        // key::end()
        // key::iterator
        {
            key k01;
            assert(k01.begin() == k01.end());

            key k02 = TEXT("\\");
            assert(k02.begin() == k02.end());

            key k03 = TEXT("\\\\");
            assert(k03.begin() == k03.end());

            key k04 = TEXT("Test");
            assert(std::distance(k04.begin(), k04.end()) == 1);
            assert(*k04.begin() == TEXT("Test"));

            key k05 = TEXT("\\Test");
            assert(std::distance(k05.begin(), k05.end()) == 1);
            assert(*k05.begin() == TEXT("Test"));

            key k06 = TEXT("Test\\");
            assert(std::distance(k06.begin(), k06.end()) == 1);
            assert(*k06.begin() == TEXT("Test"));

            key k07 = TEXT("\\\\Test\\\\");
            assert(std::distance(k07.begin(), k07.end()) == 1);
            assert(*k07.begin() == TEXT("Test"));

            key k08 = TEXT("Test1\\Test2\\Test3");
            assert(std::distance(k08.begin(), k08.end()) == 3);
            assert(*k08.begin() == TEXT("Test1"));
            assert(*(++k08.begin()) == TEXT("Test2"));
            assert(*(++(++k08.begin())) == TEXT("Test3"));

            key k09 = TEXT("\\Test1\\Test2\\Test3");
            assert(std::distance(k09.begin(), k09.end()) == 3);
            assert(*k09.begin() == TEXT("Test1"));
            assert(*(++k09.begin()) == TEXT("Test2"));
            assert(*(++(++k09.begin())) == TEXT("Test3"));

            key k10 = TEXT("Test1\\Test2\\Test3\\");
            assert(std::distance(k10.begin(), k10.end()) == 3);
            assert(*k10.begin() == TEXT("Test1"));
            assert(*(++k10.begin()) == TEXT("Test2"));
            assert(*(++(++k10.begin())) == TEXT("Test3"));

            key k11 = TEXT("\\\\Test1\\Test2\\\\Test3\\\\");
            assert(std::distance(k11.begin(), k11.end()) == 3);
            assert(*k11.begin() == TEXT("Test1"));
            assert(*(++k11.begin()) == TEXT("Test2"));
            assert(*(++(++k11.begin())) == TEXT("Test3"));

            key k12 = TEXT("Test1\\Test2\\Test3");
            auto k12_it1 = k12.begin();
            std::advance(k12_it1, 3);
            assert(k12_it1 == k12.end());

            auto k12_it2 = k12.end();
            assert(*--k12_it2 == TEXT("Test3"));
            assert(*--k12_it2 == TEXT("Test2"));
            assert(*--k12_it2 == TEXT("Test1"));
        }

        // key::has_root_key()
        {
            key k1;
            assert(k1.has_root_key() == (k1.begin() != k1.end()));

            key k2 = TEXT("Test");
            assert(k2.has_root_key() == (k2.begin() != k2.end()));

            key k3 = TEXT("Test1\\Test2\\Test3");
            assert(k3.has_root_key() == (k3.begin() != k3.end()));
        }

        // key::has_leaf_key()
        {
            key k1;
            assert(k1.has_leaf_key() == (k1.begin() != k1.end()));

            key k2 = TEXT("Test");
            assert(k2.has_leaf_key() == (k2.begin() != k2.end()));

            key k3 = TEXT("Test1\\Test2\\Test3");
            assert(k3.has_leaf_key() == (k3.begin() != k3.end()));
        }

        // key::has_parent_key()
        {
            key k1;
            assert(k1.has_parent_key() == (k1.has_root_key() && ++k1.begin() != k1.end()));

            key k2 = TEXT("Test");
            assert(k2.has_parent_key() == (k2.has_root_key() && ++k2.begin() != k2.end()));

            key k3 = TEXT("Test1\\Test2\\Test3");
            assert(k3.has_parent_key() == (k3.has_root_key() && ++k3.begin() != k3.end()));
        }

        // key::root_key()
        {
            key k1;
            assert(k1.root_key() == (k1.has_root_key() ? key(*k1.begin(), k1.view()) : key(string_type(), k1.view())));

            key k2 = TEXT("Test");
            assert(k2.root_key() == (k2.has_root_key() ? key(*k2.begin(), k2.view()) : key(string_type(), k2.view())));

            key k3 = TEXT("Test1\\Test2\\Test3");
            assert(k3.root_key() == (k3.has_root_key() ? key(*k3.begin(), k3.view()) : key(string_type(), k3.view())));
        }

        // key::leaf_key()
        {
            key k1;
            assert(k1.leaf_key() == (k1.has_leaf_key() ? key(*--k1.end(), k1.view()) : key(string_type(), k1.view())));

            key k2 = TEXT("Test");
            assert(k2.leaf_key() == (k2.has_leaf_key() ? key(*--k2.end(), k2.view()) : key(string_type(), k2.view())));

            key k3 = TEXT("Test1\\Test2\\Test3");
            assert(k3.leaf_key() == (k3.has_leaf_key() ? key(*--k3.end(), k3.view()) : key(string_type(), k3.view())));
        }

        // key::parent_key()
        {
            key k1;
            assert(k1.parent_key() == key(string_type(), k1.view()));

            key k2 = TEXT("Test");
            assert(k2.parent_key() == key(string_type(), k2.view()));

            key k3 = TEXT("Test1\\Test2\\\\Test3\\");
            assert(k3.parent_key() == key(TEXT("Test1\\Test2"), k3.view()));
        }

        // key::is_absolute()
        // key::is_relative()
        {
            key k1;
            assert(!k1.is_absolute());
            assert(k1.is_relative() == !k1.is_absolute());

            key k2 = TEXT("Test1\\Test2\\Test3");
            assert(!k2.is_absolute());
            assert(k2.is_relative() == !k2.is_absolute());

            key k3 = TEXT("\\HKEY_CURRENT_USER\\Test2\\Test3");
            assert(!k3.is_absolute());
            assert(k3.is_relative() == !k3.is_absolute());

            key k4 = TEXT("HKEY_CURRENT_USER\\Test2\\Test3");
            assert(k4.is_absolute());
            assert(k4.is_relative() == !k4.is_absolute());
        }

        // key::assign(string_view_type, view)
        {
            key k1(TEXT("Test1"), view::view_32bit);
            key k2(TEXT("Test1\\Test2\\Test3"), view::view_64bit);

            assert(k1.assign(k2.name(), k2.view()) == k2);
        }

        // key::append(string_view_type)
        {
            // redundant separator case 1
            key k1 = TEXT("HKEY_CURRENT_USER\\");
            assert(k1.append(TEXT("Test")).name() == TEXT("HKEY_CURRENT_USER\\Test"));

            // redundant separator case 2
            key k2 = TEXT("");
            key k3 = TEXT("\\\\");
            assert(k2.append(TEXT("Test")).name() == TEXT("Test"));
            assert(k3.append(TEXT("Test")).name() == TEXT("\\\\Test"));

            // redundant separator case 3
            key k4 = TEXT("HKEY_CURRENT_USER");
            assert(k4.append(TEXT("")).name() == TEXT("HKEY_CURRENT_USER"));

            // redundant separator case 4
            key k5 = TEXT("HKEY_CURRENT_USER");
            assert(k5.append(TEXT("\\Test")).name() == TEXT("HKEY_CURRENT_USER\\Test"));

            // adds a separator
            key k6 = TEXT("HKEY_CURRENT_USER");
            assert(k6.append(TEXT("Test")).name() == TEXT("HKEY_CURRENT_USER\\Test"));
        }

        // key::concat(string_view_type)
        {
            key k1, k1_copy = k1;
            assert(k1.concat(TEXT("Test")) == key(k1_copy.name() + TEXT("Test"), k1_copy.view()));

            key k2 = TEXT("HKEY_CURRENT_USER"), k2_copy = k2;
            assert(k2.concat(TEXT("Test")) == key(k2_copy.name() + TEXT("Test"), k2_copy.view()));

            key k3 = TEXT("HKEY_CURRENT_USER\\"), k3_copy = k3;
            assert(k3.concat(TEXT("Test")) == key(k3_copy.name() + TEXT("Test"), k3_copy.view()));
        }

        // key::remove_leaf()
        {
            key k1 = TEXT("HKEY_CURRENT_USER");
            assert(k1.remove_leaf().name() == TEXT(""));

            key k2 = TEXT("HKEY_CURRENT_USER\\");
            assert(k2.remove_leaf().name() == TEXT(""));

            key k3 = TEXT("\\HKEY_CURRENT_USER\\");
            assert(k3.remove_leaf().name() == TEXT(""));

            key k4 = TEXT("HKEY_CURRENT_USER\\Test");
            assert(k4.remove_leaf().name() == TEXT("HKEY_CURRENT_USER"));

            key k5 = TEXT("HKEY_CURRENT_USER\\Test\\\\");
            assert(k5.remove_leaf().name() == TEXT("HKEY_CURRENT_USER"));
        }

        // key::key::replace_leaf(string_view_type)
        {
            key k1 = TEXT("HKEY_CURRENT_USER"), k1_copy = k1;
            assert(k1.replace_leaf(TEXT("replacement")) == k1_copy.remove_leaf().append(TEXT("replacement")));

            key k2 = TEXT("HKEY_CURRENT_USER\\"), k2_copy = k2;
            assert(k2.replace_leaf(TEXT("replacement")) == k2_copy.remove_leaf().append(TEXT("replacement")));

            key k3 = TEXT("\\HKEY_CURRENT_USER\\"), k3_copy = k3;
            assert(k3.replace_leaf(TEXT("replacement")) == k3_copy.remove_leaf().append(TEXT("replacement")));

            key k4 = TEXT("HKEY_CURRENT_USER\\Test"), k4_copy = k4;
            assert(k4.replace_leaf(TEXT("replacement")) == k4_copy.remove_leaf().append(TEXT("replacement")));

            key k5 = TEXT("HKEY_CURRENT_USER\\Test\\\\"), k5_copy = k5;
            assert(k5.replace_leaf(TEXT("replacement")) == k5_copy.remove_leaf().append(TEXT("replacement")));
        }

        // key::swap
        {
            key k1(TEXT("HKEY_CURRENT_USER\\Test"), view::view_32bit),  k1_copy = k1;
            key k2(TEXT("HKEY_LOCAL_MACHINE\\Test"), view::view_64bit), k2_copy = k2;

            swap(k1, k2);
            assert(k1 == k2_copy && k2 == k1_copy);
        }

        // hash support
        {
            // TODO: ...

            /*
            key k01, k02;
            assert(hash_value(k01) == hash_value(k02));

            key k03 = TEXT("HKEY_CURRENT_USER\\Test"), k04 = k03;
            assert(hash_value(k03) == hash_value(k04));

            key k05 = TEXT("HKEY_CURRENT_USER\\Test");
            key k06 = TEXT("HKEY_CURRENT_user\\\\Test\\");
            assert(hash_value(k05) == hash_value(k06));

            key k07(TEXT("HKEY_CURRENT_USER", view::view_32bit));
            key k08(TEXT("HKEY_CURRENT_USER", view::view_64bit));
            assert(hash_value(k05) != hash_value(k06));

            key k09 = TEXT("HKEY_CURRENT_USER\\Test1");
            key k10 = TEXT("HKEY_CURRENT_USER\\\\Test1\\Test2");
            assert(hash_value(k09) != hash_value(k10));
            */
        }
    }
};

struct TEST_VALUE
{
    void operator()() noexcept
    {
        using namespace registry;

        // value::value(none_value_tag = {})
        {
            value v;
            assert(v.type() == value_type::none);
            assert(v.data().empty());
        }

        // value::value(sz_value_tag, string_view_type)
        {
            value v(sz_value_tag{}, TEXT("test"));
            assert(v.type() == value_type::sz);
            //assert(!v.data().empty());
            assert(v.to_string() == TEXT("test"));

            int exceptions = 0;
            try { v.to_uint32(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_uint64(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_strings(); }    catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 4);
        }

        // value::value(expand_sz_value_tag, string_view_type)
        {
            value v(expand_sz_value_tag{}, TEXT("test"));
            assert(v.type() == value_type::expand_sz);
            //assert(!v.data().empty());
            assert(v.to_string() == TEXT("test"));

            int exceptions = 0;
            try { v.to_uint32(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_uint64(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_strings(); }    catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 4);
        }

        // value::value(binary_value_tag, byte_array_view_type)
        {
            const std::array<uint8_t, 2> input{ 4, 2 };
            value v(binary_value_tag{}, { input.data(), input.size() });
            assert(v.type() == value_type::binary);
            assert(!v.data().empty());

            int exceptions = 0;
            try { v.to_uint32(); }  catch(bad_value_cast&) { ++exceptions; }
            try { v.to_uint64(); }  catch(bad_value_cast&) { ++exceptions; }
            try { v.to_string(); }  catch(bad_value_cast&) { ++exceptions; }
            try { v.to_strings(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 4);

            try {
                auto output = v.to_byte_array();
                assert(output.size() == input.size() && 
                       memcmp(output.data(), input.data(), input.size()) == 0);
            } catch (bad_value_cast&) {
                assert(false);
            }
        }

        // value::value(dword_value_tag, uint32_t)
        {
            value v(dword_value_tag{}, 42);
            assert(v.type() == value_type::dword);
            //assert(!v.data().empty());
            assert(v.to_uint32() == 42 && v.to_uint64() == 42);

            int exceptions = 0;
            try { v.to_string(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_strings(); }    catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 3);
        }

        // value::value(dword_big_endian_value_tag, uint32_t);
        {
            value v(dword_big_endian_value_tag{}, 42);
            assert(v.type() == value_type::dword_big_endian);
            //assert(!v.data().empty());
            assert(v.to_uint32() == 42 && v.to_uint64() == 42);

            int exceptions = 0;
            try { v.to_string(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_strings(); }    catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 3);
        }

        // value::value(link_value_tag, string_view_type)
        {
            value v(link_value_tag{}, TEXT("test"));
            assert(v.type() == value_type::link);
            //assert(!v.data().empty());
            assert(v.to_string() == TEXT("test"));

            int exceptions = 0;
            try { v.to_uint32(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_uint64(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_strings(); }    catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 4);
        }

        // value::value(multi_sz_value_tag, const Sequence&)
        {
            const std::vector<string_type> seq_in{ TEXT("test_1"), TEXT("test_2") };
            value v(multi_sz_value_tag{}, seq_in);
            assert(v.type() == value_type::multi_sz);
            //assert(!v.data().empty());
            auto seq_out = v.to_strings();
            assert(std::equal(seq_in.begin(), seq_in.end(), seq_out.begin(), seq_out.end()));

            int exceptions = 0;
            try { v.to_uint32(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_uint64(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_string(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 4);
        }

        // value::value(multi_sz_value_tag, InputIt, InputIt)
        {
            const auto seq_in = { TEXT("test_1"), TEXT("test_2") };
            value v(multi_sz_value_tag{}, seq_in.begin(), seq_in.end());
            assert(v.type() == value_type::multi_sz);
            //assert(!v.data().empty());
            auto seq_out = v.to_strings();
            assert(std::equal(seq_in.begin(), seq_in.end(), seq_out.begin(), seq_out.end()));

            int exceptions = 0;
            try { v.to_uint32(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_uint64(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_string(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 4);
        }

        // value::value(multi_sz_value_tag, std::initializer_list<String>)
        {
            const auto seq_in = { TEXT("test_1"), TEXT("test_2") };
            value v(multi_sz_value_tag{}, seq_in);
            assert(v.type() == value_type::multi_sz);
            //assert(!v.data().empty());
            auto seq_out = v.to_strings();
            assert(std::equal(seq_in.begin(), seq_in.end(), seq_out.begin(), seq_out.end()));

            int exceptions = 0;
            try { v.to_uint32(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_uint64(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_string(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 4);
        }

        // value::value(qword_value_tag, uint64_t)
        {
            value v(qword_value_tag{}, 42);
            assert(v.type() == value_type::qword);
            //assert(!v.data().empty());
            assert(v.to_uint64() == 42);

            int exceptions = 0;
            try { v.to_uint32(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_string(); }     catch(bad_value_cast&) { ++exceptions; }
            try { v.to_strings(); }    catch(bad_value_cast&) { ++exceptions; }
            try { v.to_byte_array(); } catch(bad_value_cast&) { ++exceptions; }
            assert(exceptions == 4);
        }

        // value::value(value_type, byte_array_view_type)
        {
            // TODO: ...
        }

        // ==, != operators
        {
            value v1;
            value v2(sz_value_tag{}, TEXT("test"));
            value v3(sz_value_tag{}, TEXT("test"));
            value v4(sz_value_tag{}, TEXT("test_2"));

            // a value is equal to itself
            assert(  v1 == v1 );  // testing operator ==
            assert(  v2 == v2 );
            assert(!(v1 != v1));  // testing operator !=
            assert(!(v2 != v2));

            // two identical value are equal
            assert(  v2 == v3 );  // testing operator ==
            assert(!(v2 != v3));  // testing operator !=

            // two different value are not equal
            assert(!(v1 == v2));  // testing operator ==
            assert(!(v3 == v4));
            assert(  v1 != v2 );  // testing operator !=
            assert(  v3 != v4 );
        }

        // assign(none_value_tag)
        {
            value v1(none_value_tag{});
            value v2(sz_value_tag{}, TEXT("test"));
            assert(v1 == v2.assign(none_value_tag{}));
        }

        // assign(sz_value_tag, string_view_type)
        {
            value v1(sz_value_tag{}, TEXT("test"));
            value v2;
            assert(v1 == v2.assign(sz_value_tag{}, TEXT("test")));
        }

        // assign(expand_sz_value_tag, string_view_type)
        {
            value v1(expand_sz_value_tag{}, TEXT("test"));
            value v2;
            assert(v1 == v2.assign(expand_sz_value_tag{}, TEXT("test")));
        }

        // assign(binary_value_tag, byte_array_view_type)
        {
            // TODO: ...
        }

        // assign(dword_value_tag, uint32_t)
        {
            value v1(dword_value_tag{}, 42);
            value v2;
            assert(v1 == v2.assign(dword_value_tag{}, 42));
        }

        // assign(dword_big_endian_value_tag, uint32_t)
        {
            value v1(dword_big_endian_value_tag{}, 42);
            value v2;
            assert(v1 == v2.assign(dword_big_endian_value_tag{}, 42));
        }

        // assign(link_value_tag, string_view_type)
        {
            value v1(link_value_tag{}, TEXT("test"));
            value v2;
            assert(v1 == v2.assign(link_value_tag{}, TEXT("test")));
        }

        // assign(multi_sz_value_tag, const Sequence&)
        {
            const std::vector<string_type> data{ TEXT("test_1"), TEXT("test_2") };

            value v1(multi_sz_value_tag{}, data);
            value v2;
            assert(v1 == v2.assign(multi_sz_value_tag{}, data));
        }

        // assign(multi_sz_value_tag, InputIt, InputIt)
        {
            const auto data = { TEXT("test_1"), TEXT("test_2") };

            value v1(multi_sz_value_tag{}, data.begin(), data.end());
            value v2;
            assert(v1 == v2.assign(multi_sz_value_tag{}, data.begin(), data.end()));
        }

        // assign(multi_sz_value_tag, std::initializer_list<String>)
        {
            value v1(multi_sz_value_tag{}, { TEXT("test_1"), TEXT("test_2") });
            value v2;
            assert(v1 == v2.assign(multi_sz_value_tag{}, { TEXT("test_1"), TEXT("test_2") }));
        }

        // assign(multi_sz_value_tag, const std::vector<string_view_type>&)
        {
            // TODO: ...
        }

        // assign(qword_value_tag, uint64_t value)
        {
            value v1(qword_value_tag{}, 42);
            value v2;
            assert(v1 == v2.assign(qword_value_tag{}, 42));
        }

        // assign(value_type type, byte_array_view_type data)
        {
            // TODO: ...
        }

        // value::swap
        {
            value v1(sz_value_tag{}, TEXT("test_1")),        v1_copy = v1;
            value v2(expand_sz_value_tag{}, TEXT("test_2")), v2_copy = v2;

            swap(v1, v2);
            assert(v1 == v2_copy && v2 == v1_copy);
        }
    }
};


/*
struct TEST_VALUE_ENTRY
{
    void operator()() noexcept
    {
        using namespace registry;

        // default constructor
        {
            value_entry v;
            assert(v.key().empty());
            assert(v.value_name().empty());
        }

        // value_entry::value_entry(const key&, string_view_type)
        {
            value_entry v(key_id::current_user, TEXT("test"));
            assert(v.key() == key_id::current_user);
            assert(v.value_name() == TEXT("test"));
        }

        // ==, != operators
        {
            value_entry v1;
            value_entry v2(key_id::current_user, TEXT("test"));
            value_entry v3(key_id::current_user, TEXT("test"));
            value_entry v4(key_id::local_machine, TEXT("test_2"));

            // an entry is equal to itself
            assert(  v1 == v1 );  // testing operator ==
            assert(  v2 == v2 );
            assert(!(v1 != v1));  // testing operator !=
            assert(!(v2 != v2));

            // two identical entries are equal
            assert(  v2 == v3 );  // testing operator ==
            assert(!(v2 != v3));  // testing operator !=

            // two different entries are not equal
            assert(!(v1 == v2));  // testing operator ==
            assert(!(v3 == v4));
            assert(  v1 != v2 );  // testing operator !=
            assert(  v3 != v4 );
        }

        // copy / move
        {
            value_entry v1(key_id::current_user, TEXT("test"));

            // copy construction
            value_entry v2 = v1;
            assert(v1 == v2);

            // copy assignment
            value_entry v3;
            v3 = v1;
            assert(v1 == v3);

            // move construction
            value_entry v4 = std::move(v2);
            assert(v4 == v1);

            // move assignment
            value_entry v5 = std::move(v3);
            assert(v5 == v1);
        }

        // value_entry::value()
        {
            // TODO: ...
        }

        // value_entry::value(std::error_code&)
        {
            // TODO: ...
        }

        // value_entry::swap
        {
            const value_entry ref1(key_id::current_user, TEXT("test_1"));
            const value_entry ref2(key_id::local_machine, TEXT("test_2"));

            value_entry copy1 = ref1;
            value_entry copy2 = ref2;
            swap(copy1, copy2);
            assert(copy1 == ref2 && copy2 == ref1);
        }
    }
};

struct TEST_KEY_ITERATOR
{
    void operator()() noexcept
    {
        using namespace registry;

        // default constructor
        {
            key_iterator it;
            assert(it == key_iterator());
        }

        // key_iterator::key_iterator(const key&)
        // key_iterator::key_iterator(const key&, std::error_code&)
        {
            std::error_code ec;
            const key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent");
            const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry");
            const key k2_sk = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_1_deep_0");

            key_iterator it1(k1);
            assert(it1 == key_iterator());

            key_iterator it2(k1, ec);
            assert(it2 == key_iterator() && !ec);

            key_iterator it3(k2);
            assert(it3 != key_iterator());
            assert(*it3 == k2_sk && *(it3.operator->()) == k2_sk);

            key_iterator it4(k2, ec);
            assert(it4 != key_iterator() && !ec);
            assert(*it4 == k2_sk && *(it4.operator->()) == k2_sk);
        }

        // key_iterator::operator++()
        // key_iterator::operator++(int)
        // key_iterator::increment(std::error_code&)
        {
            std::error_code ec;
            const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry");
            const key sk1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_1_deep_0");
            const key sk2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_2_deep_0");
            const key sk3 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_3_deep_0");

            key_iterator it(k);
            assert(*it == sk1);

            ++it;
            assert(*it == sk2);

            it++;
            assert(*it == sk3);

            it.increment(ec);
            assert(!ec && it == key_iterator());
        }

        // ==, != operators
        {
            key_iterator it1, it2;
            key_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
            key_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
            key_iterator it5(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_1_deep_0"));
            key_iterator it6(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_3_deep_0"));

            assert(it1 == key_iterator());
            assert(it2 == key_iterator());
            assert(it3 != key_iterator());
            assert(it4 != key_iterator());
            assert(it5 != key_iterator());
            assert(it6 != key_iterator());

            // an iterator is equal to itself
            assert(  it1 == it1 );  // testing operator ==
            assert(  it3 == it3 );
            assert(!(it1 != it1));  // testing operator !=
            assert(!(it3 != it3));

            // two empty iterators are equal
            assert(  it1 == it2 );  // testing operator ==
            assert(!(it1 != it2));  // testing operator !=

            // an empty iterator is not equal to a non-empty one
            assert(!(it1 == it3));  // testing operator ==
            assert(  it1 != it3 );  // testing operator !=

            // two non-empty identical iterators are equal
            assert(  it3 == it4 );  // testing operator ==
            assert(!(it3 != it4));  // testing operator !=

            // two different iterators are not equal
            assert(!(it4 == it5));  // testing operator ==
            assert(!(it5 == it6));
            assert(  it4 != it5 );  // testing operator !=
            assert(  it5 != it6 );
        }

        // copy / move
        {
            key_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));

            // copy construction
            key_iterator it2 = it1;
            assert(it1 == it2);

            // copy assignment
            key_iterator it3;
            it3 = it1;
            assert(it1 == it3);

            // move construction
            key_iterator it4 = std::move(it2);
            assert(it4 == it1);

            // move assignment
            key_iterator it5 = std::move(it3);
            assert(it5 == it1);
        }
    }
};

struct TEST_RECURSIVE_KEY_ITERATOR
{
    void operator()() noexcept
    {
        using namespace registry;

        // default constructor
        {
            recursive_key_iterator it;
            assert(it == recursive_key_iterator());
        }

        // recursive_key_iterator::recursive_key_iterator(const key&)
        // recursive_key_iterator::recursive_key_iterator(const key&, std::error_code&)
        {
            std::error_code ec;
            const key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent");
            const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry");
            const key k2_sk = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_1_deep_0");

            recursive_key_iterator it1(k1);
            assert(it1 == recursive_key_iterator());

            recursive_key_iterator it2(k1, ec);
            assert(it2 == recursive_key_iterator() && !ec);

            recursive_key_iterator it3(k2);
            assert(it3 != recursive_key_iterator());
            assert(*it3 == k2_sk && *(it3.operator->()) == k2_sk);
            assert(it3.depth() == 0);

            recursive_key_iterator it4(k2, ec);
            assert(it4 != recursive_key_iterator() && !ec);
            assert(*it4 == k2_sk && *(it4.operator->()) == k2_sk);
            assert(it4.depth() == 0);
        }

        // recursive_key_iterator::operator++()
        // recursive_key_iterator::operator++(int)
        // recursive_key_iterator::increment(std::error_code&)
        {
            // TODO: ...
        }

        // ==, != operators
        {
            recursive_key_iterator it1, it2;
            recursive_key_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
            recursive_key_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
            recursive_key_iterator it5(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_1_deep_0"));
            recursive_key_iterator it6(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_3_deep_0"));

            assert(it1 == recursive_key_iterator());
            assert(it2 == recursive_key_iterator());
            assert(it3 != recursive_key_iterator());
            assert(it4 != recursive_key_iterator());
            assert(it5 != recursive_key_iterator());
            assert(it6 != recursive_key_iterator());

            // an iterator is equal to itself
            assert(  it1 == it1 );  // testing operator ==
            assert(  it3 == it3 );
            assert(!(it1 != it1));  // testing operator !=
            assert(!(it3 != it3));

            // two empty iterators are equal
            assert(  it1 == it2 );  // testing operator ==
            assert(!(it1 != it2));  // testing operator !=

            // an empty iterator is not equal to a non-empty one
            assert(!(it1 == it3));  // testing operator ==
            assert(  it1 != it3 );  // testing operator !=

            // two non-empty identical iterators are equal
            assert(  it3 == it4 );  // testing operator ==
            assert(!(it3 != it4));  // testing operator !=

            // two different iterators are not equal
            assert(!(it4 == it5));  // testing operator ==
            assert(!(it5 == it6));
            assert(  it4 != it5 );  // testing operator !=
            assert(  it5 != it6 );
        }

        // copy / move
        {
            recursive_key_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));

            // copy construction
            recursive_key_iterator it2 = it1;
            assert(it1 == it2);

            // copy assignment
            recursive_key_iterator it3;
            it3 = it1;
            assert(it1 == it3);

            // move construction
            recursive_key_iterator it4 = std::move(it2);
            assert(it4 == it1);

            // move assignment
            recursive_key_iterator it5 = std::move(it3);
            assert(it5 == it1);
        }
    }
};

struct TEST_VALUE_ITERATOR
{
    void operator()() noexcept
    {
        using namespace registry;

        // default constructor
        {
            value_iterator it;
            assert(it == value_iterator());
        }

        // value_iterator::value_iterator(const key&)
        // value_iterator::value_iterator(const key&, std::error_code&)
        {
            std::error_code ec;

            value_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"));
            value_iterator it2(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existent"), ec);
            assert(it1 == value_iterator());
            assert(it2 == value_iterator() && !ec);

            value_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
            value_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), ec);
            assert(it3 != value_iterator());
            assert(it4 != value_iterator() && !ec);
        }

        // value_iterator::operator++()
        // value_iterator::operator++(int)
        // value_iterator::increment(std::error_code&)
        {
            // TODO: ...
        }

        // ==, != operators
        {
            value_iterator it1, it2;
            value_iterator it3(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
            value_iterator it4(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
            value_iterator it5(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_1_deep_0"));
            value_iterator it6(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\key_3_deep_0"));

            assert(it1 == value_iterator());
            assert(it2 == value_iterator());
            assert(it3 != value_iterator());
            assert(it4 != value_iterator());
            assert(it5 != value_iterator());
            assert(it6 != value_iterator());

            // an iterator is equal to itself
            assert(  it1 == it1 );  // testing operator ==
            assert(  it3 == it3 );
            assert(!(it1 != it1));  // testing operator !=
            assert(!(it3 != it3));

            // two empty iterators are equal
            assert(  it1 == it2 );  // testing operator ==
            assert(!(it1 != it2));  // testing operator !=

            // an empty iterator is not equal to a non-empty one
            assert(!(it1 == it3));  // testing operator ==
            assert(  it1 != it3 );  // testing operator !=

            // two non-empty identical iterators are equal
            assert(  it3 == it4 );  // testing operator ==
            assert(!(it3 != it4));  // testing operator !=

            // two different iterators are not equal
            assert(!(it4 == it5));  // testing operator ==
            assert(!(it5 == it6));
            assert(  it4 != it5 );  // testing operator !=
            assert(  it5 != it6 );
        }

        // copy / move
        {
            value_iterator it1(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));

            // copy construction
            value_iterator it2 = it1;
            assert(it1 == it2);

            // copy assignment
            value_iterator it3;
            it3 = it1;
            assert(it1 == it3);

            // move construction
            value_iterator it4 = std::move(it2);
            assert(it4 == it1);

            // move assignment
            value_iterator it5 = std::move(it3);
            assert(it5 == it1);
        }
    }
};

struct TEST_FREE_FUNCTIONS
{
    void operator()() noexcept
    {
        using namespace registry;

        // exists(const key&)
        // exists(const key&, std::error_code&)
        {
            std::error_code ec;
            assert(exists(key_id::local_machine) == true);
            assert(exists(key_id::local_machine, ec) == true && !ec);

            assert(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry")) == true);
            assert(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), ec) == true && !ec);

            assert(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\non_existent")) == false);
            assert(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\non_existent"), ec) == false && !ec);
        }

        // exists(const key&, string_view_type)
        // exists(const key&, string_view_type, std::error_code&)
        {
            std::error_code ec;
            assert(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), TEXT("val_01")) == true);
            assert(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), TEXT("val_01"), ec) == true && !ec);

            assert(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), TEXT("non_existent")) == false);
            assert(exists(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"), TEXT("non_existent"), ec) == false && !ec);
        }

        // read_value(const key&, string_view_type)
        // read_value(const key&, string_view_type, std::error_code&)
        {
            std::error_code ec;
            const key k(TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry"));
            
            auto v01  = read_value(k, TEXT("val_01"));
            auto v01a = read_value(k, TEXT("val_01"), ec);
            assert(!ec && v01 == v01a);

            auto v02  = read_value(k, TEXT("val_02"));
            auto v02a = read_value(k, TEXT("val_02"), ec);
            assert(!ec && v02 == v02a);

            auto v03  = read_value(k, TEXT("val_03"));
            auto v03a = read_value(k, TEXT("val_03"), ec);
            assert(!ec && v03 == v03a);

            auto v04  = read_value(k, TEXT("val_04"));
            auto v04a = read_value(k, TEXT("val_04"), ec);
            assert(!ec && v04 == v04a);

            auto v05  = read_value(k, TEXT("val_05"));
            auto v05a = read_value(k, TEXT("val_05"), ec);
            assert(!ec && v05 == v05a);

            auto v06  = read_value(k, TEXT("val_06"));
            auto v06a = read_value(k, TEXT("val_06"), ec);
            assert(!ec && v06 == v06a);

            auto v07  = read_value(k, TEXT("val_07"));
            auto v07a = read_value(k, TEXT("val_07"), ec);
            assert(!ec && v07 == v07a);

            auto v08  = read_value(k, TEXT("val_08"));
            auto v08a = read_value(k, TEXT("val_08"), ec);
            assert(!ec && v08 == v08a);

            auto v09  = read_value(k, TEXT("val_09"));
            auto v09a = read_value(k, TEXT("val_09"), ec);
            assert(!ec && v09 == v09a);

            auto v10 = read_value(k, TEXT("non_existent"), ec);
            assert(ec && v10 == value());

            int exceptions = 0;
            try {
                read_value(k, TEXT("non_existent"));
            } catch(const registry_error& e) {
                ++exceptions;
                assert(e.key1() == k);
                assert(e.key2().empty());
                assert(e.value_name() == TEXT("non_existent"));
            }
            assert(exceptions == 1);
            
            assert(v01.type() == value_type::none             && v01.data().size() == 0);
            assert(v02.type() == value_type::sz               && v02.to_string() == TEXT("42"));
            assert(v03.type() == value_type::expand_sz        && v03.to_string() == TEXT("42"));
            assert(v04.type() == value_type::binary           && (v04.to_byte_array() == byte_array_type{ 4, 2 }));
            assert(v05.type() == value_type::dword            && v05.to_uint32() == 42);
            assert(v06.type() == value_type::dword_big_endian && v06.to_uint32() == 42);
            assert(v07.type() == value_type::link             && v07.to_string() == TEXT("42"));
            assert(v08.type() == value_type::multi_sz         && (v08.to_strings() == 
                                                                  std::vector<string_type>{ TEXT("42"), TEXT("42") }));
            assert(v09.type() == value_type::qword            && v09.to_uint64() == 42);
        }

        // create_key(const key& key)
        // create_key(const key& key, std::error_code&)
        {
            std::error_code ec;
            const key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1");
            const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_2");

            assert(!exists(k1) && !exists(k2));
            assert(create_key(k1) == true && exists(k1));
            assert(create_key(k2, ec) == true && !ec && exists(k2));

            assert(create_key(key_id::current_user) == false);
            assert(create_key(key_id::current_user, ec) == false && !ec);

            assert(create_key(k1) == false);
            assert(create_key(k2, ec) == false && !ec);
        }

        // create_keys(const key& key)
        // create_keys(const key& key, std::error_code&)
        {
            std::error_code ec;
            key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_3");
            key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_4");
            
            assert(!exists(k1) && !exists(k2));
            assert(create_keys(k1.append(TEXT("Inner1\\Inner2"))) == true && exists(k1));
            assert(create_keys(k2.append(TEXT("Inner1\\Inner2")), ec) == true && !ec && exists(k2));

            assert(create_keys(key_id::current_user) == false);
            assert(create_keys(key_id::current_user, ec) == false && !ec);

            assert(create_keys(k1) == false);
            assert(create_keys(k2, ec) == false && !ec);
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
            assert((write_value(k, TEXT("val_01a"), v01, ec), !ec));
            assert((write_value(k, TEXT("val_02a"), v02, ec), !ec));
            assert((write_value(k, TEXT("val_03a"), v03, ec), !ec));
            assert((write_value(k, TEXT("val_04a"), v04, ec), !ec));
            assert((write_value(k, TEXT("val_05a"), v05, ec), !ec));
            assert((write_value(k, TEXT("val_06a"), v06, ec), !ec));
            assert((write_value(k, TEXT("val_07a"), v07, ec), !ec));
            assert((write_value(k, TEXT("val_08a"), v08, ec), !ec));
            assert((write_value(k, TEXT("val_09a"), v09, ec), !ec));

            assert(read_value(k, TEXT("val_01")) == v01 && read_value(k, TEXT("val_01a")) == v01);
            assert(read_value(k, TEXT("val_02")) == v02 && read_value(k, TEXT("val_02a")) == v02);
            assert(read_value(k, TEXT("val_03")) == v03 && read_value(k, TEXT("val_03a")) == v03);
            assert(read_value(k, TEXT("val_04")) == v04 && read_value(k, TEXT("val_04a")) == v04);
            assert(read_value(k, TEXT("val_05")) == v05 && read_value(k, TEXT("val_05a")) == v05);
            assert(read_value(k, TEXT("val_06")) == v06 && read_value(k, TEXT("val_06a")) == v06);
            assert(read_value(k, TEXT("val_07")) == v07 && read_value(k, TEXT("val_07a")) == v07);
            assert(read_value(k, TEXT("val_08")) == v08 && read_value(k, TEXT("val_08a")) == v08);
            assert(read_value(k, TEXT("val_09")) == v09 && read_value(k, TEXT("val_09a")) == v09);
        }

        // remove(const key&, string_view_type)
        // remove(const key&, string_view_type, std::error_code&)
        {
            std::error_code ec;
            const key k = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1");

            assert(!exists(k, TEXT("non_existing")));
            assert(exists(k, TEXT("val_01")) && exists(k, TEXT("val_02")));

            // remove an non-existing value
            assert(remove(k, TEXT("non_existing")) == false);
            assert(remove(k, TEXT("non_existing"), ec) == false && !ec);

            // remove an existing value
            assert(remove(k, TEXT("val_01")) == true && !exists(k, TEXT("val_01")));
            assert(remove(k, TEXT("val_02"), ec) == true && !ec && !exists(k, TEXT("val_02")));
        }

        // remove(const key&)
        // remove(const key&, std::error_code&)
        {
            std::error_code ec;
            const key k0 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existing");
            const key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_1");
            const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_2");
            const key k3 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_3");

            assert(!exists(k0) && exists(k1) && exists(k2));
            assert(exists(key(k3).append(TEXT("Inner1\\Inner2"))));

            // remove an non-existing key
            assert(remove(k0) == false);
            assert(remove(k0, ec) == false && !ec);

            // remove an empty key (with no subkeys)
            assert(remove(k1) == true && !exists(k1));
            assert(remove(k2, ec) == true && !ec && !exists(k2));

            // try remove an non-empty key (which have subkeys)
            int exception = 0;
            assert(remove(k3, ec) == false && ec && exists(k3));
            try {
                remove(k3);
            } catch (const registry_error& e) {
                ++exception;
                exists(k3);
            }
            assert(exception == 1);
        }

        // remove_all(const key&)
        // remove_all(const key&, std::error_code&)
        {
            std::error_code ec;
            const key k0 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\non_existing");
            const key k1 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_3");
            const key k2 = TEXT("HKEY_CURRENT_USER\\SOFTWARE\\libregistry\\new_key_4");

            assert(!exists(k0));
            assert(exists(key(k1).append(TEXT("Inner1\\Inner2"))));
            assert(exists(key(k2).append(TEXT("Inner1\\Inner2"))));

            // remove an non-existing key
            assert(remove_all(k0) == 0);
            assert(remove_all(k0, ec) == 0 && !ec);

            // remove an non-empty key (which have subkeys)
            assert(remove_all(k1) == 3 && !exists(k1));
            assert(remove_all(k2, ec) == 3 && !ec && !exists(k2));
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
            assert((is_64bit_machine && ans == false) || (!is_64bit_machine && ans == true));
        }
    }
};
*/

int main()
{
    //prepare_stage();
    //BOOST_SCOPE_EXIT_ALL(&) { clear_stage(); };

    try {
        TEST_REGISTRY_ERROR()();
        TEST_KEY()();
        TEST_VALUE()();
        //TEST_VALUE_ENTRY()();
        //TEST_KEY_ITERATOR()();
        //TEST_RECURSIVE_KEY_ITERATOR()();
        //TEST_VALUE_ITERATOR()();
        //TEST_FREE_FUNCTIONS()();
    } catch (...) {
        assert(false);
    }
    return 0;
}