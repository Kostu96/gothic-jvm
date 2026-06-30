#pragma once

class VM;
class Frame;

namespace com::nokia::mid::ui {

class FullCanvas {
public:
    static void init(VM& vm, Frame& frame);

    static void get_width(VM& vm, Frame& frame);
    static void get_height(VM& vm, Frame& frame);
};

}
