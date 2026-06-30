#include "native_classes/io/data_input_stream.hpp"

#include "runtime/frame.hpp"

namespace java::io {

void DataInputStream::init(VM& vm, Frame& frame) {
    frame.pop_stack(); // this
}

}
