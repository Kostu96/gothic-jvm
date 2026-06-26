#include "runtime/vm.hpp"

#include "runtime/class.hpp"

#include <stdexcept>

VM::VM(std::string_view main_class) :
    main_class_(main_class),
    interpreter_(*this)
{
    class_loader_.load_native("java/lang/Object");
    class_loader_.load_native("java/util/Random");
}

void VM::add_classpath_entry(std::filesystem::path dir) {
    class_loader_.add_classpath_entry(std::move(dir));
}

void VM::run() {
    Class* main = load_class(main_class_);
    initialize_class(main);

    // TODO: look up main([Ljava/lang/String;)V on `main` and invoke it via
    // interpreter_.execute(main, *main_method, args).
}

Class* VM::load_class(std::string_view binary_name) {
    return class_loader_.load(binary_name);
}

void VM::initialize_class(Class* cls) {
    if (!cls) {
        return;
    }

    switch (cls->init_state()) {
    case ClassInitState::Initialized:
    case ClassInitState::Initializing: // re-entrant from this thread is a no-op (§5.5)
        return;
    case ClassInitState::Failed:
        throw std::runtime_error(
            "VM: class '" + std::string(cls->name()) + "' previously failed initialization");
    case ClassInitState::Loaded:
        break;
    }

    cls->set_init_state(ClassInitState::Initializing);

    // TODO: load the super class via class_loader_ and recursively initialize it
    // before running this class's <clinit>.

    try {
        if (const Method* clinit = cls->find_method("<clinit>", "()V")) {
            interpreter_.execute(cls, *clinit);
        }
    }
    catch (...) {
        cls->set_init_state(ClassInitState::Failed);
        throw;
    }

    cls->set_init_state(ClassInitState::Initialized);
}
