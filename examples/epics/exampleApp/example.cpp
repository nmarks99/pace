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
    // std::string protocol;
    // if (argc < 2) {
        // std::cout << "Usage: ./example <protocol>\n";
        // return EXIT_FAILURE;
    // }
    // protocol = std::string(argv[1]);

    // std::cout << "Using " << protocol << " protocol" << std::endl;
    ezec::Context ctxt("ca"); // default protocol = Channel Access
    ctxt.add({
        "nmarks:m1.DESC",
        "ca://nmarks:m1.RBV",
        "ca://nmarks:m1.VAL",
        "pva://nmarks:m1.PREC",
    });

    double rbv = 0.0;
    ctxt.bind(rbv, "nmarks:m1.RBV");
    double val = 0.0;
    ctxt.bind(val, "nmarks:m1.VAL");
    std::string desc;
    ctxt.bind(desc, "nmarks:m1.DESC");
    double posx;
    ctxt.bind(posx, "pva://nmarks:m1.PREC");

    auto d = ctxt["nmarks:m1.DESC"].peek<std::string>();
    if (d) {
        std::cout << *d << std::endl;
    } else {
        std::cout << "d is not readable as a std::string (yet)\n";
    }

    while (!g_signal_caught) {
        if (ctxt.sync()) {
            std::cout << "m1.VAL = " << rbv << std::endl;
            std::cout << "m1.RBV = " << val << std::endl;
            std::cout << "m1.DESC = " << desc << std::endl;
            std::cout << "m1.PREC = " << posx << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\nExiting.\n";
    return 0;
}
