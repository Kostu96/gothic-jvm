#include "native_classes/lang/clazz.hpp"

#include "runtime/frame.hpp"
#include "runtime/runtime_object.hpp"

#include <stdexcept>

namespace java::lang {

void Class::get_resource_as_stream(VM& vm, Frame& frame) {
    // Stack: ..., this (Class mirror), name (String)
    // Resource loading is not implemented yet: it requires String modeling and
    // classpath resource lookup returning a java/io/InputStream.
    throw std::runtime_error(
        "java/lang/Class.getResourceAsStream: not implemented");
}

}
