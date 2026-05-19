#include <iostream>
#include <csignal>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include "ezec.hpp"

volatile std::sig_atomic_t g_signal_caught = 0;
void signal_handler(int) { g_signal_caught = 1; }

struct Motor {
    Motor(const std::string& name) :
        val(name + ".VAL"),
        rbv(name + ".RBV"),
        spmg(name + ".SPMG"),
        prec(name + ".PREC") {}

    ezec::CAChannel val;
    ezec::CAChannel rbv;
    ezec::CAChannel spmg;
    ezec::CAChannel prec;
};

int main(int argc, char* argv[]) {

    std::signal(SIGINT, signal_handler);
    // if (argc < 2) {
        // std::cerr << "Usage: example <pv_name> [pv_name ...]\n";
        // return 1;
    // }

    ezec::Context ctx;
    Motor motor("nick:m1");

    double rbv = 0.0;
    motor.rbv.bind(rbv);

    int spmg = 0.0;
    motor.spmg.bind(spmg);

    int prec = 0;
    motor.prec.bind(prec);

    while (!g_signal_caught) {
        if (motor.rbv.sync()) {
            std::cout << rbv << std::endl;
        }
        if (motor.spmg.sync()) {
            std::cout << spmg << std::endl;
        }
        if (motor.prec.sync()) {
            std::cout << "PREC = " << prec << std::endl;
        }

        if (motor.prec.connected()) {
            if (prec != 7) {
                motor.prec.put(7);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\nExiting.\n";
    return 0;
}
