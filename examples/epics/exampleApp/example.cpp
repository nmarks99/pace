#include <iostream>
#include <csignal>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include "ezec.hpp"

volatile std::sig_atomic_t g_signal_caught = 0;
void signal_handler(int) { g_signal_caught = 1; }

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: example <pv_name> [pv_name ...]\n";
        return 1;
    }

    std::signal(SIGINT, signal_handler);

    std::vector<std::string> pv_names;
    for (int i = 1; i < argc; ++i) {
        pv_names.emplace_back(argv[i]);
    }

    ezec::Context ctxt;
    ezec::ChannelGroup group;

    struct PVData {
        std::string name;
        double val_double = 0.0;
        int val_int = 0;
        std::string val_string;
    };

    std::vector<PVData> pvs(pv_names.size());
    for (size_t i = 0; i < pv_names.size(); ++i) {
        pvs[i].name = pv_names[i];
        group.add(pv_names[i]);
        group.bind(pvs[i].val_double, pv_names[i]);
        group.bind(pvs[i].val_int, pv_names[i]);
        group.bind(pvs[i].val_string, pv_names[i]);
    }

    std::cout << "Waiting for connections...\n";
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!g_signal_caught && std::chrono::steady_clock::now() < deadline) {
        bool all_connected = true;
        for (auto& pv : pvs) {
            if (!group.get_channel(pv.name).connected()) {
                all_connected = false;
                break;
            }
        }
        if (all_connected) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    for (auto& pv : pvs) {
        bool conn = group.get_channel(pv.name).connected();
        std::cout << pv.name << ": " << (conn ? "connected" : "NOT connected") << "\n";
    }
    std::cout << "\nMonitoring (Ctrl+C to quit)...\n\n";

    while (!g_signal_caught) {
        if (group.sync()) {
            for (auto& pv : pvs) {
                std::cout << pv.name << "  double=" << pv.val_double
                          << "  int=" << pv.val_int
                          << "  string=\"" << pv.val_string << "\"\n";
            }
            std::cout << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\nExiting.\n";
    return 0;
}
