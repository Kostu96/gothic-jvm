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

Object* Heap::new_interned_string(std::string_view str) {
    std::string value(str);
    if (auto it = string_objects_.find(value); it != string_objects_.end()) {
        return it->second;
    }

    Object* string_obj = new_instance(*string_class_);
    string_objects_.emplace(value, string_obj);

    auto& instance = std::get<InstanceData>(string_obj->data);
    const auto set_field = [&](std::string_view name, std::string_view descriptor, Value field_value) {
        if (auto field = string_class_->find_field(name, descriptor)) {
            instance.fields[field->slot] = field_value;
        }
    };
    set_field("count", "I", static_cast<int32_t>(str.size()));
    set_field("hash", "I", static_cast<int32_t>(0)); // TODO(Kostu): hash is not set properly

    instance.native_payload = StringNativeData{ value };

    return string_obj;
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
