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

    ezec::Context ctxt("ca");

    ezec::CAChannel m1_rbv("nmarks:m1.RBV");

    double rbv = 0.0;
    std::string rbv_str;
    double rbv2 = 0.0;

    m1_rbv.bind(rbv);
    m1_rbv.bind(rbv_str);
    m1_rbv.bind(rbv2);

    while (g_signal_caught == 0) {
        if (m1_rbv.sync()) {
            std::cout << "rbv=" << rbv
                      << " str=" << rbv_str
                      << " rbv2=" << rbv2 << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}
