#include "native_classes/lang/object.hpp"

#include "runtime/class.hpp"
#include "runtime/frame.hpp"
#include "runtime/runtime_object.hpp"
#include "runtime/vm.hpp"

#include <stdexcept>
#include <variant>

namespace java::lang {

void Object::get_class(VM& vm, Frame& frame) {
    RuntimeObject* self = std::get<RuntimeObject*>(frame.pop_stack());
    if (self == nullptr) {
        // In a complete VM this would raise NullPointerException.
        throw std::runtime_error("Object.getClass: null self");
    }

    Class* runtime_class = nullptr;
    if (auto* instance = std::get_if<InstanceData>(&self->data)) {
        runtime_class = instance->type;
    }
    else {
        // getClass() on array and java/lang/Class mirror receivers needs
        // synthetic/reflective class resolution that is not yet wired up.
        throw std::runtime_error("Object.getClass: unsupported receiver kind");
    }

    frame.push_stack(vm.heap().class_object_for(*runtime_class));
}

}
