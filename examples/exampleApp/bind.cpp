#include <iostream>
#include <csignal>
#include <string>
#include <thread>
#include <chrono>

#include "pace.hpp"

// Used to catch ctrl+c and fail gracefully.
volatile std::sig_atomic_t g_signal_caught = 0;
void signal_handler(int) { g_signal_caught = 1; }

int main(int argc, char* argv[]) {

    std::signal(SIGINT, signal_handler);

    // IOC prefix
    if (argc != 2) {
        std::cout << "Please provide IOC prefix\n";
        return 1;
    }
    auto P = std::string(argv[1]);

    pace::Context ctxt;

    double rbv = 0.0;
    std::string desc;
    ctxt.connect(P+"m1.RBV").bind(rbv);
    ctxt.connect(P+"m1.DESC").bind(desc);

    while (!g_signal_caught) {
        if (ctxt.sync()) {
            std::cout << desc << " = " << rbv << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Exiting\n";

    return 0;
}
