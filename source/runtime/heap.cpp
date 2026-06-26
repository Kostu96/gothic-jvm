#include "runtime/heap.hpp"

#include <stdexcept>
#include <utility>

ArrayObject* Heap::new_array(ArrayType element_type, int32_t length) {
    if (length < 0) {
        // In a complete VM this would raise NegativeArraySizeException.
        throw std::runtime_error("Heap: negative array size");
    }

    auto array = std::make_unique<ArrayObject>(element_type, length);
    ArrayObject* reference = array.get();
    objects_.push_back(std::move(array));

    return reference;
}

InstanceObject* Heap::new_instance(Class* type) {
    auto instance = std::make_unique<InstanceObject>(type);
    InstanceObject* reference = instance.get();
    objects_.push_back(std::move(instance));

    return reference;
}
