#include "class_loader/class_file.hpp"
#include "class_loader/class_loader.hpp"
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
    ClassLoader class_loader;
    Class hello(test_file("HelloWorld.class").c_str(), class_loader);

    EXPECT_EQ(hello.this_name(), "HelloWorld");
    EXPECT_EQ(hello.super_name(), "java/lang/Object");
}

TEST(ClassFileTest, HelloWorldContainsInitAndMain) {
    ClassLoader class_loader;
    Class hello(test_file("HelloWorld.class").c_str(), class_loader);

    EXPECT_NE(hello.find_method("<init>", "()V"), nullptr);

    const Method* main_method = hello.find_method("main", "([Ljava/lang/String;)V");
    ASSERT_NE(main_method, nullptr);
    EXPECT_FALSE(main_method->code.empty());
    EXPECT_GT(main_method->max_stack, 0u);
}

TEST(ClassFileTest, HelloWorldFindMethodReturnsNullForMiss) {
    ClassLoader class_loader;
    Class hello(test_file("HelloWorld.class").c_str(), class_loader);

    EXPECT_EQ(hello.find_method("doesNotExist", "()V"), nullptr);
    EXPECT_EQ(hello.find_method("main", "()V"), nullptr); // wrong descriptor
}

TEST(ClassFileTest, ParsesAdditionClassName) {
    ClassLoader class_loader;
    Class addition(test_file("Addition.class").c_str(), class_loader);

    EXPECT_EQ(addition.this_name(), "Addition");
    EXPECT_EQ(addition.super_name(), "java/lang/Object");
}

TEST(ClassFileTest, AdditionContainsAllMethods) {
    ClassLoader class_loader;
    Class addition(test_file("Addition.class").c_str(), class_loader);

    EXPECT_NE(addition.find_method("<init>", "()V"), nullptr);
    // Static initializer is emitted because of `static int x = 3; static int y = 5;`.
    EXPECT_NE(addition.find_method("<clinit>", "()V"), nullptr);
    EXPECT_NE(addition.find_method("addLiterals", "()I"), nullptr);
    EXPECT_NE(addition.find_method("addFields", "()I"), nullptr);
    EXPECT_NE(addition.find_method("addArguments", "(II)I"), nullptr);
    EXPECT_NE(addition.find_method("main", "([Ljava/lang/String;)V"), nullptr);
}

TEST(ClassFileTest, AdditionAddArgumentsHasNonEmptyCode) {
    ClassLoader class_loader;
    Class addition(test_file("Addition.class").c_str(), class_loader);

    const Method* method = addition.find_method("addArguments", "(II)I");
    ASSERT_NE(method, nullptr);
    EXPECT_FALSE(method->code.empty());
    // Two int parameters => at least 2 local slots.
    EXPECT_GE(method->max_locals, 2u);
}

TEST(ClassFileTest, MissingFileThrows) {
    ClassLoader class_loader;
    EXPECT_THROW(Class("definitely_not_a_real_path_to.class", class_loader), std::runtime_error);
}
