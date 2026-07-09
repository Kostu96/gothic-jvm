#include "runtime/heap.hpp"

#include <stdexcept>
#include <utility>

Object* Heap::new_instance(Class& type) {
    auto instance = std::make_unique<Object>(InstanceData{ type });

    auto& data = std::get<InstanceData>(instance->data);
    data.fields.resize(type.instance_field_count());

    Object* reference = instance.get();
    objects_.push_back(std::move(instance));

    return reference;
}

Object* Heap::new_primitive_array(PrimitiveArrayData::ElementType element_type, int32_t length) {
    if (length < 0) {
        // In a complete VM this would raise NegativeArraySizeException.
        throw std::runtime_error("Heap: negative array size");
    }

    auto array = std::make_unique<Object>(PrimitiveArrayData(element_type, length));
    Object* reference = array.get();
    objects_.push_back(std::move(array));

    return reference;
}

Object* Heap::new_instance_array(Class& element_type, int32_t length) {
    if (length < 0) {
        // In a complete VM this would raise NegativeArraySizeException.
        throw std::runtime_error("Heap: negative array size");
    }

    auto array = std::make_unique<Object>(InstanceArrayData(element_type, length));
    Object* reference = array.get();
    objects_.push_back(std::move(array));

    return reference;
}

Object* Heap::class_object_for(Class& mirrored) {
    if (auto it = class_objects_.find(&mirrored); it != class_objects_.end()) {
        return it->second;
    }

    auto mirror = std::make_unique<Object>(ClassMirrorData{ mirrored });
    Object* reference = mirror.get();
    objects_.push_back(std::move(mirror));
    class_objects_.emplace(&mirrored, reference);

    return reference;
}
