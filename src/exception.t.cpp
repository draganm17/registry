#include <Windows.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <registry/exception.h>
#include <registry/key_path.h>

using namespace registry;


TEST(RegistryError, Construct)
{
    // registry_error::registry_error(std::error_code, const std::string&)
    {
        const auto ec = std::make_error_code(std::errc::not_enough_memory);
        registry_error ex(ec, "test");

        EXPECT_TRUE(ex.path1() == key_path());
        EXPECT_TRUE(ex.path2() == key_path());
        EXPECT_TRUE(ex.value_name().empty());
    }

    // registry_error::registry_error(std::error_code, const std::string&, const key_path&)
    {
        const auto ec = std::make_error_code(std::errc::not_enough_memory);
        registry_error ex(ec, "test", key_path::from_key_id(key_id::current_user));

        EXPECT_TRUE(ex.path1() == key_path::from_key_id(key_id::current_user));
        EXPECT_TRUE(ex.path2() == key_path());
        EXPECT_TRUE(ex.value_name().empty());
    }

    // registry_error::registry_error(std::error_code, const std::string&, const key_path&, const key_path&)
    {
        const auto ec = std::make_error_code(std::errc::not_enough_memory);
        registry_error ex(ec, "test", 
                          key_path::from_key_id(key_id::current_user), 
                          key_path::from_key_id(key_id::local_machine));

        EXPECT_TRUE(ex.path1() == key_path::from_key_id(key_id::current_user));
        EXPECT_TRUE(ex.path2() == key_path::from_key_id(key_id::local_machine));
        EXPECT_TRUE(ex.value_name().empty());
    }

    // registry_error::registry_error(std::error_code, const std::string&, 
    //                                const key_path&, const key_path&, string_view_type)
    {
        const auto ec = std::make_error_code(std::errc::not_enough_memory);
        registry_error ex(ec, "test", key_path::from_key_id(key_id::current_user),
                          key_path::from_key_id(key_id::local_machine), TEXT("test"));

        EXPECT_TRUE(ex.path1() == key_path::from_key_id(key_id::current_user));
        EXPECT_TRUE(ex.path2() == key_path::from_key_id(key_id::local_machine));
        EXPECT_TRUE(ex.value_name() == TEXT("test"));
    }
}