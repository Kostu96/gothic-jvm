set(TEST_SOURCES
    "test_binary_reader.cpp"
    "test_class_file.cpp"
)

add_executable(app_tests ${TEST_SOURCES})

target_link_libraries(app_tests PRIVATE app_lib gtest_main)

target_compile_definitions(app_tests PRIVATE
    TEST_FILES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../resources/test_files"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX tests FILES ${TEST_SOURCES})

include(GoogleTest)
gtest_discover_tests(app_tests)
