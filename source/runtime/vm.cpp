#include "runtime/vm.hpp"

#include "runtime/class.hpp"

#include <print>
#include <stdexcept>

VM::VM(std::string_view main_class) :
    main_class_(main_class),
    interpreter_(*this)
{
    //class_loader_.load_native("java/io/DataInputStream");
    //class_loader_.load_native("java/lang/Object");
    //class_loader_.load_native("java/lang/Class");
    //class_loader_.load_native("java/lang/String");
    //class_loader_.load_native("java/util/Hashtable");
    //class_loader_.load_native("java/util/Random");
    //class_loader_.load_native("java/util/Stack");
    //class_loader_.load_native("java/util/Vector");
    //class_loader_.load_native("javax/microedition/midlet/MIDlet");
    //class_loader_.load_native("com/nokia/mid/ui/FullCanvas");
}

void VM::add_classpath_entry(std::filesystem::path dir) {
    class_loader_.add_classpath_entry(std::move(dir));
}

void VM::run() {
    Class* main = load_class(main_class_);
    initialize_class(*main);

    RuntimeObject* main_obj = heap_.new_instance(*main);
    const auto init = main->find_method("<init>", "()V");
    Value main_obj_value = main_obj;
    interpreter_.execute(*main, *init, std::span{ &main_obj_value, 1 });

    const auto start_app = main->find_method("startApp", "()V");
    interpreter_.execute(*main, *start_app);
}

Class* VM::load_class(std::string_view binary_name) {
    return class_loader_.load(binary_name);
}

void VM::initialize_class(Class& cls) {
    switch (cls.init_state()) {
    case ClassInitState::Initialized:
    case ClassInitState::Initializing: // re-entrant from this thread is a no-op (§5.5)
        return;
    case ClassInitState::Failed:
        throw std::runtime_error(
            "VM: class '" + std::string(cls.this_name()) + "' previously failed initialization");
    case ClassInitState::Loaded:
        break;
    }

    cls.set_init_state(ClassInitState::Initializing);

    try {
        if (const std::string_view super = cls.super_name(); !super.empty()) {
            initialize_class(*load_class(super));
        }

        if (const Method* clinit = cls.find_method("<clinit>", "()V")) {
            interpreter_.execute(cls, *clinit);
        }
    }
    catch (...) {
        cls.set_init_state(ClassInitState::Failed);
        throw;
    }

    cls.set_init_state(ClassInitState::Initialized);
}
