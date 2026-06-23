cmake_minimum_required(VERSION 3.28)

project(gothic-jvm LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(APP_LIB_SOURCES
    "source/runtime/class.hpp"
    "source/runtime/class.cpp"
    "source/utils/binary_reader.cpp"
    "source/utils/binary_reader.hpp"
    "source/class_file.cpp"
    "source/class_file.hpp"
)

add_library(app_lib STATIC ${APP_LIB_SOURCES})
target_compile_features(app_lib PUBLIC cxx_std_23)
target_include_directories(app_lib PUBLIC source)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX source FILES ${APP_LIB_SOURCES})

add_executable(app "source/main.cpp")
target_link_libraries(app PRIVATE app_lib)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX source FILES "source/main.cpp")

# --- Tests ---
enable_testing()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/gtest)

# Tidy gtest targets into a folder in IDEs that support solution folders.
foreach(_gt_target IN ITEMS gtest gtest_main)
    if(TARGET ${_gt_target})
        set_target_properties(${_gt_target} PROPERTIES FOLDER "third_party")
    endif()
endforeach()

add_subdirectory(tests)
