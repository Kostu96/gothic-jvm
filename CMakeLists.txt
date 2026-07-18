cmake_minimum_required(VERSION 3.28)

project(gothic-jvm LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# SDL3 provides the window, 2D renderer and event loop the MIDlet screen is
# drawn with. Build it as a shared library and skip its tests/examples to keep
# configure/build times down.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

add_subdirectory(java_classes)

set(APP_LIB_SOURCES
    "source/class_loader/class_file.cpp"
    "source/class_loader/class_file.hpp"
    "source/class_loader/class_loader.cpp"
    "source/class_loader/class_loader.hpp"
    "source/class_loader/constant_pool_entry.hpp"
    "source/platform/display.cpp"
    "source/platform/display.hpp"
    "source/platform/record_store.cpp"
    "source/platform/record_store.hpp"
    "source/platform/record_store_manager.cpp"
    "source/platform/record_store_manager.hpp"
    "source/runtime/class.hpp"
    "source/runtime/class.cpp"
    "source/runtime/frame.hpp"
    "source/runtime/frame.cpp"
    "source/runtime/heap.hpp"
    "source/runtime/heap.cpp"
    "source/runtime/interpreter.hpp"
    "source/runtime/interpreter.cpp"
    "source/runtime/native_methods.hpp"
    "source/runtime/native_methods.cpp"
    "source/runtime/opcodes.hpp"
    "source/runtime/runtime_constant_pool_entry.hpp"
    "source/runtime/object.hpp"
    "source/runtime/object.cpp"
    "source/runtime/thread.cpp"
    "source/runtime/thread.hpp"
    "source/runtime/value.hpp"
    "source/runtime/vm.hpp"
    "source/runtime/vm.cpp"
    "source/utils/binary_reader.cpp"
    "source/utils/binary_reader.hpp"
)

add_library(app_lib STATIC ${APP_LIB_SOURCES})
target_compile_features(app_lib PUBLIC cxx_std_23)
target_include_directories(app_lib PUBLIC source)
target_link_libraries(app_lib PUBLIC SDL3::SDL3)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX source FILES ${APP_LIB_SOURCES})

add_executable(app "source/main.cpp")
target_link_libraries(app PRIVATE app_lib)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX source FILES "source/main.cpp")

add_dependencies(app java_classes)

# SDL3 is a shared library, so its runtime DLL has to sit next to the binaries
# that link it (both the app and the test runner).
if(WIN32)
    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:app>"
        VERBATIM)
endif()

enable_testing()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/gtest)

set_target_properties(gtest gtest_main PROPERTIES FOLDER "third_party")

add_subdirectory(tests)
