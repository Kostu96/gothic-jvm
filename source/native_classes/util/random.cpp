#include "native_classes/util/random.hpp"

#include "runtime/frame.hpp"

namespace java::util {

void Random::init(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
}

}
