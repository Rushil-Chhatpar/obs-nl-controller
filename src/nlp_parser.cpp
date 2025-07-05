#include "nlp_parser.hpp"
#include <algorithm>
#include <iostream>

NLPParser::NLPParser(OBSController& controller) : controller_(controller) {
    command_map_ = {
        {"start recording", [&](std::string) { controller_.sendCommand("StartRecord"); }},
        {"stop recording", [&](std::string) { controller_.sendCommand("StopRecord"); }},
        {"switch to scene", [&](std::string scene) {
            if (!scene.empty()) {
                controller_.sendCommand("SetCurrentProgramScene", {{"sceneName", scene}});
            } else {
                std::cerr << "No scene name provided" << std::endl;
            }
        }}
    };
}

void NLPParser::processCommand(const std::string& input) {
    std::string lowercase = input;
    std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
    
    for (const auto& [command, action] : command_map_) {
        if (lowercase.find(command) != std::string::npos) {
            if (command == "switch to scene") {
                // Extract scene name (assume it follows "switch to scene")
                size_t pos = lowercase.find("switch to scene");
                std::string scene = lowercase.substr(pos + 15);
                scene.erase(0, scene.find_first_not_of(" "));
                scene.erase(scene.find_last_not_of(" ") + 1);
                action(scene);
            } else {
                action(""); // No parameter needed for other commands
            }
            return;
        }
    }
    std::cerr << "Unknown command: " << input << std::endl;
}