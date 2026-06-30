#pragma once

class VM;
class Frame;

namespace java::lang {

class Class {
public:
    static void get_resource_as_stream(VM& vm, Frame& frame);
};

}
