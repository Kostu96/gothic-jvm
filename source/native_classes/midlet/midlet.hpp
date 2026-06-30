#pragma once

class VM;
class Frame;

namespace javax::microedition::midlet {

class MIDlet {
public:
    static void init(VM& vm, Frame& frame);
};

}
