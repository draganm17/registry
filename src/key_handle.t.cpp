#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key_handle.h>

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

TEST(KeyHandle, Getters)
{
    key_handle h01 = key_id::classes_root;
    EXPECT_TRUE(h01.valid());
    EXPECT_TRUE(h01.key() == key::from_key_id(key_id::classes_root));
    EXPECT_TRUE(h01.rights() == access_rights::unknown);
    EXPECT_TRUE(h01.native_handle() == static_cast<key_handle::native_handle_type>(key_id::classes_root));

    key_handle h02 = key_id::current_config;
    EXPECT_TRUE(h02.valid());
    EXPECT_TRUE(h02.key() == key::from_key_id(key_id::current_config));
    EXPECT_TRUE(h02.rights() == access_rights::unknown);
    EXPECT_TRUE(h02.native_handle() == static_cast<key_handle::native_handle_type>(key_id::current_config));

    key_handle h03 = key_id::current_user;
    EXPECT_TRUE(h03.valid());
    EXPECT_TRUE(h03.key() == key::from_key_id(key_id::current_user));
    EXPECT_TRUE(h03.rights() == access_rights::unknown);
    EXPECT_TRUE(h03.native_handle() == static_cast<key_handle::native_handle_type>(key_id::current_user));

    key_handle h04 = key_id::current_user_local_settings;
    EXPECT_TRUE(h04.valid());
    EXPECT_TRUE(h04.key() == key::from_key_id(key_id::current_user_local_settings));
    EXPECT_TRUE(h04.rights() == access_rights::unknown);
    EXPECT_TRUE(h04.native_handle() == static_cast<key_handle::native_handle_type>(key_id::current_user_local_settings));

    key_handle h05 = key_id::current_user_local_settings;
    EXPECT_TRUE(h05.valid());
    EXPECT_TRUE(h05.key() == key::from_key_id(key_id::current_user_local_settings));
    EXPECT_TRUE(h05.rights() == access_rights::unknown);
    EXPECT_TRUE(h05.native_handle() == static_cast<key_handle::native_handle_type>(key_id::current_user_local_settings));

    key_handle h06 = key_id::local_machine;
    EXPECT_TRUE(h06.valid());
    EXPECT_TRUE(h06.key() == key::from_key_id(key_id::local_machine));
    EXPECT_TRUE(h06.rights() == access_rights::unknown);
    EXPECT_TRUE(h06.native_handle() == static_cast<key_handle::native_handle_type>(key_id::local_machine));

    key_handle h07 = key_id::performance_data;
    EXPECT_TRUE(h07.valid());
    EXPECT_TRUE(h07.key() == key::from_key_id(key_id::performance_data));
    EXPECT_TRUE(h07.rights() == access_rights::unknown);
    EXPECT_TRUE(h07.native_handle() == static_cast<key_handle::native_handle_type>(key_id::performance_data));

    key_handle h08 = key_id::performance_nlstext;
    EXPECT_TRUE(h08.valid());
    EXPECT_TRUE(h08.key() == key::from_key_id(key_id::performance_nlstext));
    EXPECT_TRUE(h08.rights() == access_rights::unknown);
    EXPECT_TRUE(h08.native_handle() == static_cast<key_handle::native_handle_type>(key_id::performance_nlstext));

    key_handle h09 = key_id::performance_text;
    EXPECT_TRUE(h09.valid());
    EXPECT_TRUE(h09.key() == key::from_key_id(key_id::performance_text));
    EXPECT_TRUE(h09.rights() == access_rights::unknown);
    EXPECT_TRUE(h09.native_handle() == static_cast<key_handle::native_handle_type>(key_id::performance_text));

    key_handle h10 = key_id::unknown;
    EXPECT_TRUE(!h10.valid());
    EXPECT_TRUE(h10.key() == key());
    EXPECT_TRUE(h10.rights() == access_rights::unknown);
    EXPECT_TRUE(h10.native_handle() == key_handle::native_handle_type{});
}