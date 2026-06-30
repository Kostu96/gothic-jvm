#pragma once

class VM;
class Frame;

namespace java::io {

class DataInputStream {
public:
    static void init(VM& vm, Frame& frame);
};

}
