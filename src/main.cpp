#include "obs_controller.hpp"
#include "nlp_parser.hpp"
#include <iostream>
#include <thread>

int main() {
    OBSController controller("ws://localhost:4455", "x826ciLsU5FJucvP"); // Update with OBS WebSocket password
    std::thread ws_thread([&controller]() {
        if (!controller.connect()) {
            std::cerr << "Failed to connect to OBS WebSocket" << std::endl;
            return;
        }
    });

    // Wait for connection and authentication
    std::this_thread::sleep_for(std::chrono::seconds(2));

    NLPParser parser(controller);
    std::string input;
    std::cout << "Enter command (e.g., 'start recording', 'switch to scene Main'): ";
    while (std::getline(std::cin, input) && input != "quit") {
        parser.processCommand(input);
        std::cout << "Enter command: ";
    }

    controller.disconnect();
    ws_thread.join();
    return 0;
}