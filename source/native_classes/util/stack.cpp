#include "native_classes/util/stack.hpp"

#include "runtime/frame.hpp"

namespace java::util {

void Stack::init(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
}

}
