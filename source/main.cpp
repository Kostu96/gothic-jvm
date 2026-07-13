#include "platform/display.hpp"
#include "runtime/vm.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <print>
#include <thread>

/*
 * args <main_class> [<class_path_entry>...]
 */
int main(int argc, char** argv) {
    try {
        const char* main_class = argc >= 2 ? argv[1] : "HG";

        Display display("gothic-jvm", 240, 320, 2);

        VM vm;
        vm.set_display(&display);

        if (argc >= 3) {
            for (int i = 2; i < argc; ++i) {
                vm.class_loader().add_classpath_entry(argv[i]);
            }
        }
        else {
            vm.class_loader().add_classpath_entry(std::filesystem::current_path() / "gothic3thebeginning");
        }

        // Run the JVM off the main thread: SDL must own the thread that created
        // the window and pumps its events, and a MIDlet's startApp() may never
        // return. The window loop below drives events (and, in the future,
        // painting the current Canvas).
        std::thread jvm_thread([&] {
            try {
                vm.run(main_class);
            }
            catch (const VmStopRequested&) {
                // Window closed while the MIDlet was running; unwind quietly.
            }
            catch (const std::exception& e) {
                std::println(stderr, "{}", e.what());
            }
        });

        while (display.process_events()) {
            display.clear(0, 0, 0);
            display.present();
        }

        // Ask the interpreter to unwind, then wait for the JVM thread to finish.
        vm.request_stop();
        jvm_thread.join();
    }
    catch (const std::exception& e) {
        std::println(stderr, "{}", e.what());
        return 1;
    }

    return 0;
}
