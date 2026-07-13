set(TEST_SOURCES
    "test_binary_reader.cpp"
)

add_executable(app_tests ${TEST_SOURCES})

target_link_libraries(app_tests PRIVATE app_lib gtest_main)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX tests FILES ${TEST_SOURCES})

# app_lib links SDL3 as a shared library, so the test runner needs the DLL too.
if(WIN32)
    add_custom_command(TARGET app_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:app_tests>"
        VERBATIM)
endif()

include(GoogleTest)
gtest_discover_tests(app_tests)
