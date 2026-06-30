#include "native_classes/util/vector.hpp"

#include "runtime/frame.hpp"

namespace java::util {

void Vector::init(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
}

}
