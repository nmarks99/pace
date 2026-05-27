#include <iostream>
#include <csignal>
#include <string>
#include <thread>
#include <chrono>

#include "ezec.hpp"

// Used to catch ctrl+c and fail gracefully.
volatile std::sig_atomic_t g_signal_caught = 0;
void signal_handler(int) { g_signal_caught = 1; }

int main(int argc, char* argv[]) {

    std::signal(SIGINT, signal_handler);

    // User specifies protocol "ca" or "pva" from command line
    std::string protocol;
    if (argc < 2) {
        std::cout << "Usage: ./example <protocol>\n";
        return EXIT_FAILURE;
    }
    protocol = std::string(argv[1]);

    std::cout << "Using " << protocol << " protocol" << std::endl;
    ezec::Context ctxt(protocol);

    double rbv = 0.0;
    ctxt.add("nmarks:m1.RBV").bind(rbv);
    double val = 0.0;
    ctxt.add("nmarks:m1.VAL").bind(val);
    std::string desc;
    ctxt.add("nmarks:m1.DESC").bind(desc);

    while (!g_signal_caught) {
        if (ctxt.sync()) {
            std::cout << "VAL = " << rbv << std::endl;
            std::cout << "RBV = " << val << std::endl;
            std::cout << "DESC = " << desc << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\nExiting.\n";
    return 0;
}
