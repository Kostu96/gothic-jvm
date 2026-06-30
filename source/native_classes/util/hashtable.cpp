#include "native_classes/util/hashtable.hpp"

#include "runtime/frame.hpp"

namespace java::util {

void Hashtable::init(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
}

}
