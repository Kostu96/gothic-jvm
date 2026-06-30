#include "native_classes/lang/string.hpp"

#include "runtime/frame.hpp"

namespace java::lang {

void String::init(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
}

}
