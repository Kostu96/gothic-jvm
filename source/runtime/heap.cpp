#include "runtime/heap.hpp"

#include <stdexcept>
#include <utility>

RuntimeObject* Heap::new_instance(Class& type) {
    auto instance = std::make_unique<RuntimeObject>(RuntimeObject{ InstanceData{ &type } });
    RuntimeObject* reference = instance.get();
    objects_.push_back(std::move(instance));

    return reference;
}

RuntimeObject* Heap::new_primitive_array(ElementType element_type, int32_t length) {
    if (length < 0) {
        // In a complete VM this would raise NegativeArraySizeException.
        throw std::runtime_error("Heap: negative array size");
    }

    auto array = std::make_unique<RuntimeObject>(RuntimeObject{ PrimitiveArrayData(element_type, length) });
    RuntimeObject* reference = array.get();
    objects_.push_back(std::move(array));

    return reference;
}

RuntimeObject* Heap::new_instance_array(Class& element_type, int32_t length) {
    if (length < 0) {
        // In a complete VM this would raise NegativeArraySizeException.
        throw std::runtime_error("Heap: negative array size");
    }

    auto array = std::make_unique<RuntimeObject>(RuntimeObject{ InstanceArrayData(element_type, length) });
    RuntimeObject* reference = array.get();
    objects_.push_back(std::move(array));

    return reference;
}

RuntimeObject* Heap::class_object_for(Class& mirrored) {
    if (auto it = class_objects_.find(&mirrored); it != class_objects_.end()) {
        return it->second;
    }

    auto mirror = std::make_unique<RuntimeObject>(RuntimeObject{ ClassMirrorData{ &mirrored } });
    RuntimeObject* reference = mirror.get();
    objects_.push_back(std::move(mirror));
    class_objects_.emplace(&mirrored, reference);

    return reference;
}
