set(TEST_SOURCES
    "test_binary_reader.cpp"
)

add_executable(app_tests ${TEST_SOURCES})

target_link_libraries(app_tests PRIVATE app_lib gtest_main)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX tests FILES ${TEST_SOURCES})

include(GoogleTest)
gtest_discover_tests(app_tests)
