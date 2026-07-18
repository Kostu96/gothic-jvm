#include "runtime/vm.hpp"

#include "runtime/class.hpp"

#include <print>
#include <stdexcept>

VM::VM() :
    class_loader_(native_methods_),
    interpreter_(*this)
{
    class_loader_.add_classpath_entry(std::filesystem::current_path() / "java_classes");
}

void VM::run() {
    enum class BootsrapState {
        Boot,
        Phase1,
        Phase2,
        Phase3,
        Phase4
    };
    BootsrapState bootstrap_state = BootsrapState::Boot;
    auto& main_thread = *threads_.emplace_back(std::make_unique<Thread>()).get();

    Class& string_class = class_loader_.load("java/lang/String");
    heap_.set_string_class(string_class);
    Class& main_class = class_loader_.load(main_class_name_);
    Value main_obj;

    while (true) {
        switch (bootstrap_state) {
            using enum BootsrapState;
        case Boot: {
            bootstrap_state = Phase1;
            string_class.ensure_initialized(main_thread);
        } break;
        case Phase1: {
            if (main_thread.is_terminated()) {
                bootstrap_state = Phase2;
                main_class.ensure_initialized(main_thread);
            }
        } break;
        case Phase2: {
            if (main_thread.is_terminated()) {
                bootstrap_state = Phase3;
                main_obj = heap_.new_instance(main_class);
                main_thread.push_frame(*main_class.find_method("<init>", "()V"), std::span{ &main_obj, 1 });
            }
        } break;
        case Phase3: {
            if (main_thread.is_terminated()) {
                bootstrap_state = Phase4;
                main_thread.push_frame(*main_class.find_method("startApp", "()V"), std::span{ &main_obj, 1 });
            }
        } break;
        case Phase4: {
            if (main_thread.is_terminated()) {
                return;
            }
        } break;
        }

        interpreter_.run(main_thread, 500);
    }
}
