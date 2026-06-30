#pragma once

class VM;
class Frame;

namespace java::lang {

class String {
public:
    static void init(VM& vm, Frame& frame);
};

}
