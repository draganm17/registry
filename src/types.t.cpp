#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/key.h>
#include <registry/types.h>

using namespace registry;


TEST(RegistryError, Construct)
{
    // registry_error::registry_error(std::error_code, const std::string&)
    {
        const auto ec = std::make_error_code(std::errc::not_enough_memory);
        registry_error ex(ec, "test");

        EXPECT_TRUE(ex.key1() == key());
        EXPECT_TRUE(ex.key2() == key());
        EXPECT_TRUE(ex.value_name().empty());
    }

    // registry_error::registry_error(std::error_code, const std::string&, const key&)
    {
        const auto ec = std::make_error_code(std::errc::not_enough_memory);
        registry_error ex(ec, "test", key::from_key_id(key_id::current_user));

        EXPECT_TRUE(ex.key1() == key::from_key_id(key_id::current_user));
        EXPECT_TRUE(ex.key2() == key());
        EXPECT_TRUE(ex.value_name().empty());
    }

    // registry_error::registry_error(std::error_code, const std::string&, const key&, const key&)
    {
        const auto ec = std::make_error_code(std::errc::not_enough_memory);
        registry_error ex(ec, "test", key::from_key_id(key_id::current_user), key::from_key_id(key_id::local_machine));

        EXPECT_TRUE(ex.key1() == key::from_key_id(key_id::current_user));
        EXPECT_TRUE(ex.key2() == key::from_key_id(key_id::local_machine));
        EXPECT_TRUE(ex.value_name().empty());
    }

    // registry_error::registry_error(std::error_code, const std::string&, 
    //                                const key&, const key&, string_view_type)
    {
        const auto ec = std::make_error_code(std::errc::not_enough_memory);
        registry_error ex(ec, "test", key::from_key_id(key_id::current_user), 
                          key::from_key_id(key_id::local_machine), TEXT("test"));

        EXPECT_TRUE(ex.key1() == key::from_key_id(key_id::current_user));
        EXPECT_TRUE(ex.key2() == key::from_key_id(key_id::local_machine));
        EXPECT_TRUE(ex.value_name() == TEXT("test"));
    }
}