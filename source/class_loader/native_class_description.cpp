#include "native_class_description.hpp"

#include "native_classes/io/data_input_stream.hpp"
#include "native_classes/util/hashtable.hpp"
#include "native_classes/util/random.hpp"
#include "native_classes/util/stack.hpp"
#include "native_classes/util/vector.hpp"

#include "native_classes/midlet/midlet.hpp"

#include "native_classes/ui/full_canvas.hpp"

#include "native_classes/lang/object.hpp"
#include "native_classes/lang/clazz.hpp"

NativeClassDescription NativeClassDescription::data_input_stream() {
    NativeClassDescription description{};
    description.name = "java/io/DataInputStream";

    return description;
}

NativeClassDescription NativeClassDescription::object() {
    NativeClassDescription description{};
    description.name = "java/lang/Object";
    description.super_name = {};

    description.methods.push_back(NativeMethod{ "getClass", "()Ljava/lang/Class;", java::lang::Object::get_class });

    return description;
}

NativeClassDescription NativeClassDescription::clazz() {
    NativeClassDescription description{};
    description.name = "java/lang/Class";

    description.methods.push_back(NativeMethod{
        "getResourceAsStream", "(Ljava/lang/String;)Ljava/io/InputStream;",
        java::lang::Class::get_resource_as_stream });

    return description;
}

NativeClassDescription NativeClassDescription::hashtable() {
    NativeClassDescription description{};
    description.name = "java/util/Hashtable";

    description.methods.push_back(NativeMethod{ "<init>", "()V", java::util::Hashtable::init });

    return description;
}

NativeClassDescription NativeClassDescription::random() {
    NativeClassDescription description{};
    description.name = "java/util/Random";

    description.methods.push_back(NativeMethod{ "<init>", "()V", java::util::Random::init });

    return description;
}

NativeClassDescription NativeClassDescription::stack() {
    NativeClassDescription description{};
    description.name = "java/util/Stack";

    description.methods.push_back(NativeMethod{ "<init>", "()V", java::util::Stack::init });

    return description;
}

NativeClassDescription NativeClassDescription::string() {
    NativeClassDescription description{};
    description.name = "java/lang/String";

    return description;
}

NativeClassDescription NativeClassDescription::vector() {
    NativeClassDescription description{};
    description.name = "java/util/Vector";

    description.methods.push_back(NativeMethod{ "<init>", "()V", java::util::Vector::init });

    return description;
}

NativeClassDescription NativeClassDescription::midlet() {
    NativeClassDescription description{};
    description.name = "javax/microedition/midlet/MIDlet";

    description.methods.push_back(NativeMethod{ "<init>", "()V", javax::microedition::midlet::MIDlet::init });

    return description;
}

NativeClassDescription NativeClassDescription::full_canvas() {
    NativeClassDescription description{};
    description.name = "com/nokia/mid/ui/FullCanvas";

    description.methods.push_back(NativeMethod{ "<init>", "()V", com::nokia::mid::ui::FullCanvas::init });
    description.methods.push_back(NativeMethod{ "getWidth", "()I", com::nokia::mid::ui::FullCanvas::get_width });
    description.methods.push_back(NativeMethod{ "getHeight", "()I", com::nokia::mid::ui::FullCanvas::get_height });

    return description;
}
