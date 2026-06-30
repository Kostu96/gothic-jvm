#pragma once

class VM;
class Frame;

namespace java::util {

class Stack {
public:
    static void init(VM& vm, Frame& frame);
};

}
