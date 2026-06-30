#pragma once

class VM;
class Frame;

namespace java::util {

class Random {
public:
    static void init(VM& vm, Frame& frame);
};

}
