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

    // IOC prefix
    std::string P = "pva://nick:";

    ezec::Context ctxt("pva");
    ctxt.add(P+"Position");
    // double x, y, z = 0.0;
    // ctxt.add(P+"Position").bind(x, "X.value");
    // ctxt.add(P+"Position").bind(y, "Y.value");
    // ctxt.add(P+"Position").bind(z, "Z.value");

    // Hacky, waiting for PVs to connect
    std::cout << "Waiting to connect\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "Trying get operations\n";
    auto x = ctxt.get<double>(P+"Position", "X.value");
    if (x) {
        std::cout << "x = " << x.value() << std::endl;
    }
//
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
