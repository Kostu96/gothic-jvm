#include "native_classes/midlet/midlet.hpp"

#include "runtime/frame.hpp"

namespace javax::microedition::midlet {

void MIDlet::init(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
}

}
