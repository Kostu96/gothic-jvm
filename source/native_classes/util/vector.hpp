#pragma once

class VM;
class Frame;

namespace java::util {

class Vector {
public:
    static void init(VM& vm, Frame& frame);
};

}
