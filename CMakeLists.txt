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
#    "source/class_loader/native_class_description.cpp"
#    "source/class_loader/native_class_description.hpp"
#    "source/native_classes/io/data_input_stream.cpp"
#    "source/native_classes/io/data_input_stream.hpp"
#    "source/native_classes/lang/clazz.cpp"
#    "source/native_classes/lang/clazz.hpp"
#    "source/native_classes/lang/object.cpp"
#    "source/native_classes/lang/object.hpp"
#    "source/native_classes/lang/string.cpp"
#    "source/native_classes/lang/string.hpp"
#    "source/native_classes/midlet/midlet.cpp"
#    "source/native_classes/midlet/midlet.hpp"
#    "source/native_classes/ui/full_canvas.cpp"
#    "source/native_classes/ui/full_canvas.hpp"
#    "source/native_classes/util/hashtable.cpp"
#    "source/native_classes/util/hashtable.hpp"
#    "source/native_classes/util/random.cpp"
#    "source/native_classes/util/random.hpp"
#    "source/native_classes/util/stack.cpp"
#    "source/native_classes/util/stack.hpp"
#    "source/native_classes/util/vector.cpp"
#    "source/native_classes/util/vector.hpp"
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
    "source/runtime/runtime_object.hpp"
    "source/runtime/runtime_object.cpp"
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
