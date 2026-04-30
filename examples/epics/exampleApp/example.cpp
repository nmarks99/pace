#include <iostream>
#include <csignal>
#include <string>
#include <thread>
#include <chrono>

#include "ezec.hpp"

volatile std::sig_atomic_t g_signal_caught = 0;
void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nKeyboard interrupt (Ctrl+C)" << std::endl;
        g_signal_caught = 1;
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);

    // Get PV name from command line args
    if (argc != 2) {
        std::cout << "Usage: example <pv name>\n";
        return EXIT_SUCCESS;
    }
    const std::string pv_name = argv[1];

    // Create Channel Access context
    ezec::Context ctxt;

    ezec::ChannelGroup group;
    group.add(pv_name);

    // User variables to store PV value from monitor
    std::string val_string;
    double val_double = 0.0;

    // Bind user variables to monitor
    group.bind(val_string, pv_name);
    group.bind(val_double, pv_name);

    while (g_signal_caught == 0) {

        // If there is new data, print it
        if (group.sync()) {
            std::cout << pv_name + "[string] = " << val_string << "\n";
            std::cout << pv_name + "[double] = " << val_double << "\n\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}
