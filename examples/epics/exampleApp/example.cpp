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

    std::string P = "nmarks:";

    ezec::Context ctxt;
    ctxt.add(P+"m1.RBV").monitor();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    double rbv = 0.0;
    ctxt.bind(rbv, P+"m1.RBV");

    while (!g_signal_caught) {
        if (ctxt.sync()) {
            if (auto rbv_op = ctxt.peek<double>(P+"m1.RBV"); rbv_op) {
                std::cout << "Peeked RBV = " << *rbv_op << std::endl;
            }

            std::cout << "Bound RBV = " << rbv << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\nExiting.\n";
    return 0;
}
