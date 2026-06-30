#pragma once

class VM;
class Frame;

namespace java::util {

class Hashtable {
public:
    static void init(VM& vm, Frame& frame);
};

}
