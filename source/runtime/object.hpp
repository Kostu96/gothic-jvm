#pragma once
#include "runtime/value.hpp"

#include <cstdint>
#include <vector>

class Class;

class Object {
public:
    enum class Kind {
        Array,
        Instance,
    };

    explicit Object(Kind kind) noexcept : kind_(kind) {}
    virtual ~Object() = default;

    Kind kind() const noexcept { return kind_; }

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
private:
    Kind kind_;
};

enum class ArrayType : uint8_t {
    Boolean = 4,
    Char = 5,
    Float = 6,
    Double = 7,
    Byte = 8,
    Short = 9,
    Int = 10,
    Long = 11,
};

class ArrayObject : public Object {
public:
    ArrayObject(ArrayType element_type, int32_t length);

    ArrayType element_type() const noexcept { return element_type_; }
    int32_t length() const noexcept { return elements_.size(); }

    Value get(int32_t index) const;
    void set(int32_t index, Value value);
private:
    ArrayType element_type_;
    std::vector<Value> elements_;
};

// An ordinary object created by the `new` opcode. It only carries its type for
// now; per-instance field storage will be added alongside putfield/getfield.
class InstanceObject : public Object {
public:
    explicit InstanceObject(Class* type) noexcept
        : Object(Kind::Instance), type_(type) {}

    Class* type() const noexcept { return type_; }
private:
    Class* type_;
};
