#pragma once

class VM;
class Frame;

namespace java::lang {

class Object {
public:
    static void get_class(VM& vm, Frame& frame);
};

}
