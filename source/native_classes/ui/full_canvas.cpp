#include "native_classes/ui/full_canvas.hpp"

#include "runtime/frame.hpp"

namespace {

constexpr int32_t FULL_CANVAS_WIDTH = 240;
constexpr int32_t FULL_CANVAS_HEIGHT = 320;

}

namespace com::nokia::mid::ui {

void FullCanvas::init(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
}

void FullCanvas::get_width(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
    frame.push_stack(FULL_CANVAS_WIDTH);
}

void FullCanvas::get_height(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
    frame.push_stack(FULL_CANVAS_HEIGHT);
}

}
