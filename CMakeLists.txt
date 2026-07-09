cmake_minimum_required(VERSION 3.28)

project(gothic-jvm LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(APP_LIB_SOURCES
    "source/class_loader/class_file.cpp"
    "source/class_loader/class_file.hpp"
    "source/class_loader/class_loader.cpp"
    "source/class_loader/class_loader.hpp"
    "source/class_loader/constant_pool_entry.hpp"
    "source/runtime/class.hpp"
    "source/runtime/class.cpp"
    "source/runtime/frame.hpp"
    "source/runtime/frame.cpp"
    "source/runtime/heap.hpp"
    "source/runtime/heap.cpp"
    "source/runtime/interpreter.hpp"
    "source/runtime/interpreter.cpp"
    "source/runtime/opcodes.hpp"
    "source/runtime/runtime_constant_pool_entry.hpp"
    "source/runtime/object.hpp"
    "source/runtime/object.cpp"
    "source/runtime/value.hpp"
    "source/runtime/vm.hpp"
    "source/runtime/vm.cpp"
    "source/utils/binary_reader.cpp"
    "source/utils/binary_reader.hpp"
)

add_library(app_lib STATIC ${APP_LIB_SOURCES})
target_compile_features(app_lib PUBLIC cxx_std_23)
target_include_directories(app_lib PUBLIC source)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX source FILES ${APP_LIB_SOURCES})

add_executable(app "source/main.cpp")
target_link_libraries(app PRIVATE app_lib)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX source FILES "source/main.cpp")

# Copy *.class files from java_classes into the build tree (mirroring the directory
# structure) after building app. Existing files are replaced; others are left as is.
file(GLOB_RECURSE APP_CLASS_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/java_classes
    ${CMAKE_CURRENT_SOURCE_DIR}/java_classes/*.class)
foreach(_class_file IN LISTS APP_CLASS_FILES)
    add_custom_command(TARGET app POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/java_classes/${_class_file}
            ${CMAKE_BINARY_DIR}/java_classes/${_class_file}
        VERBATIM
    )
endforeach()

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
