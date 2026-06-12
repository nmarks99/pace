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
    std::string P = "nmarks:";

    pace::Context ctxt;
    ctxt.connect("pva://"+P+"Position", 1.0);
    ctxt.connect(P+"m1.RBV", 1.0);
    ctxt.connect(P+"m1.DESC", 1.0);
    ctxt.connect(P+"m1.VAL", 1.0);

    std::cout << "Trying get operations\n";

    if (auto desc = ctxt.get<std::string>(P+"m1.DESC")) {
        std::cout << "DESC = " << *desc << std::endl;
    }

    if (auto rbv = ctxt.get<double>(P+"m1.RBV")) {
        std::cout << "RBV = " << *rbv << std::endl;
    }

    if (auto x = ctxt.get<double>(P+"Position", "X.value")) {
        std::cout << "X = " << *x << std::endl;
    }
    if (auto y = ctxt.get<double>(P+"Position", "Y.value")) {
        std::cout << "Y = " << *y << std::endl;
    }
    if (auto z = ctxt.get<double>(P+"Position", "Z.value")) {
        std::cout << "Z = " << *z << std::endl;
    }

    ctxt[P+"m1.DESC"] = "Motor 1";
    ctxt[P+"m1.VAL"] = 3.14;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (auto desc = ctxt.get<std::string>(P+"m1.DESC")) {
        std::cout << "DESC = " << *desc << std::endl;
    }

    if (auto rbv = ctxt.get<double>(P+"m1.RBV")) {
        std::cout << "RBV = " << *rbv << std::endl;
    }

    // while (!g_signal_caught) {
        // if (ctxt.sync()) {
            // std::cout << "X = " << x << std::endl;
            // std::cout << "Y = " << y << std::endl;
            // std::cout << "Z = " << z << std::endl;
        // }
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }
    std::cout << "\nExiting.\n";
    return 0;
}
