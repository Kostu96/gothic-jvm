#include "runtime/vm.hpp"

#include "runtime/class.hpp"

#include <print>
#include <stdexcept>

VM::VM(std::string_view main_class) :
    main_class_(main_class),
    interpreter_(*this)
{
    class_loader_.add_classpath_entry(std::filesystem::current_path() / "java_classes");
    class_loader_.load("java/lang/Class");
    class_loader_.load("java/lang/String");
}

void VM::run() {
    Class& main = class_loader_.load(main_class_);
    initialize_class(main);

    Object* main_obj = heap_.new_instance(main);
    const auto init = main.find_method("<init>", "()V");
    Value main_obj_value = main_obj;
    interpreter_.execute(*init, std::span{ &main_obj_value, 1 });

    const auto start_app = main.find_method("startApp", "()V");
    interpreter_.execute(*start_app, std::span{ &main_obj_value, 1 });
}

void VM::initialize_class(Class& cls) {
    switch (cls.init_state()) {
    case Class::InitState::Initialized:
    case Class::InitState::Initializing:
        return;
    case Class::InitState::Failed:
        throw std::runtime_error(
            "VM: class '" + std::string(cls.this_name()) + "' previously failed initialization");
    case Class::InitState::Loaded:
        break;
    }

    std::println("Initializing class: {}", cls.this_name());
    cls.set_init_state(Class::InitState::Initializing);

    try {
        if (const std::string_view super = cls.super_name(); !super.empty()) {
            initialize_class(class_loader().load(super));
        }

        if (const Method* clinit = cls.find_method("<clinit>", "()V")) {
            interpreter_.execute(*clinit);
        }
    }
    catch (...) {
        cls.set_init_state(Class::InitState::Failed);
        throw;
    }

    cls.set_init_state(Class::InitState::Initialized);
}
