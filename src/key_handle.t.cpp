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

TEST(KeyHandle, Factories)
{
    // TODO: ...
}

TEST(KeyHandle, GettersAndQueries)
{
    static const auto test = [](key_id id)
    {
        key_handle h = id;
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