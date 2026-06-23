#include "class_file.hpp"
#include "runtime/class.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace {

std::string test_file(const char* name) {
    return (std::filesystem::path(TEST_FILES_DIR) / name).string();
}

} // namespace

TEST(ClassFileTest, ParsesHelloWorldClassName) {
    Class hello(test_file("HelloWorld.class").c_str());

    EXPECT_EQ(hello.name(), "HelloWorld");
    EXPECT_EQ(hello.super_name(), "java/lang/Object");
}

TEST(ClassFileTest, HelloWorldContainsInitAndMain) {
    Class hello(test_file("HelloWorld.class").c_str());

    EXPECT_NE(hello.find_method("<init>", "()V"), nullptr);

    const Method* main_method = hello.find_method("main", "([Ljava/lang/String;)V");
    ASSERT_NE(main_method, nullptr);
    EXPECT_FALSE(main_method->code.empty());
    EXPECT_GT(main_method->max_stack, 0u);
}

TEST(ClassFileTest, HelloWorldFindMethodReturnsNullForMiss) {
    Class hello(test_file("HelloWorld.class").c_str());

    EXPECT_EQ(hello.find_method("doesNotExist", "()V"), nullptr);
    EXPECT_EQ(hello.find_method("main", "()V"), nullptr); // wrong descriptor
}

TEST(ClassFileTest, ParsesAdditionClassName) {
    Class addition(test_file("Addition.class").c_str());

    EXPECT_EQ(addition.name(), "Addition");
    EXPECT_EQ(addition.super_name(), "java/lang/Object");
}

TEST(ClassFileTest, AdditionContainsAllMethods) {
    Class addition(test_file("Addition.class").c_str());

    EXPECT_NE(addition.find_method("<init>", "()V"), nullptr);
    // Static initializer is emitted because of `static int x = 3; static int y = 5;`.
    EXPECT_NE(addition.find_method("<clinit>", "()V"), nullptr);
    EXPECT_NE(addition.find_method("addLiterals", "()I"), nullptr);
    EXPECT_NE(addition.find_method("addFields", "()I"), nullptr);
    EXPECT_NE(addition.find_method("addArguments", "(II)I"), nullptr);
    EXPECT_NE(addition.find_method("main", "([Ljava/lang/String;)V"), nullptr);
}

TEST(ClassFileTest, AdditionAddArgumentsHasNonEmptyCode) {
    Class addition(test_file("Addition.class").c_str());

    const Method* method = addition.find_method("addArguments", "(II)I");
    ASSERT_NE(method, nullptr);
    EXPECT_FALSE(method->code.empty());
    // Two int parameters => at least 2 local slots.
    EXPECT_GE(method->max_locals, 2u);
}

TEST(ClassFileTest, MissingFileThrows) {
    EXPECT_THROW(Class("definitely_not_a_real_path_to.class"), std::runtime_error);
}
