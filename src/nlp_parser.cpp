#include "nlp_parser.hpp"
#include <algorithm>
#include <iostream>
#include <regex>

NLPParser::NLPParser(OBSController& controller) : controller_(controller) {
    command_map_ = {
        {"start recording", [&](std::string) { controller_.sendCommand("StartRecord"); }},
        {"stop recording", [&](std::string) { controller_.sendCommand("StopRecord"); }},
        {"switch to scene", [&](std::string scene) {
            if (!scene.empty()) {
                controller_.sendCommand("SetCurrentProgramScene", {{"sceneName", scene}});
            } else {
                std::cerr << "Error: No scene name provided for 'switch to scene'" << std::endl;
            }
        }}
    };
}

void NLPParser::processCommand(const std::string& input) {
    // Create lowercase copy for command matching
    std::string lowercase = input;
    std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), ::tolower);
    
    // Trim leading/trailing whitespace
    lowercase.erase(lowercase.begin(), std::find_if(lowercase.begin(), lowercase.end(), [](unsigned char c) { return !std::isspace(c); }));
    lowercase.erase(std::find_if(lowercase.rbegin(), lowercase.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), lowercase.end());

    // Use regex to extract scene name, preserving original case
    std::regex scene_regex("switch to scene\\s+(.+)", std::regex::icase);
    std::smatch match;
    
    for (const auto& [command, action] : command_map_) {
        if (lowercase.find(command) != std::string::npos) {
            if (command == "switch to scene") {
                if (std::regex_match(input, match, scene_regex)) { // Use original input for case sensitivity
                    std::string scene = match[1].str();
                    // Trim scene name
                    scene.erase(scene.begin(), std::find_if(scene.begin(), scene.end(), [](unsigned char c) { return !std::isspace(c); }));
                    scene.erase(std::find_if(scene.rbegin(), scene.rend(), [](unsigned char c) { return !std::isspace(c); }).base(), scene.end());
                    if (!scene.empty()) {
                        action(scene);
                    } else {
                        std::cerr << "Error: No scene name provided in 'switch to scene'" << std::endl;
                    }
                } else {
                    std::cerr << "Error: Invalid 'switch to scene' command format. Use: switch to scene <name>" << std::endl;
                }
            } else {
                action(""); // No parameter needed for other commands
            }
            return;
        }
    }
    std::cerr << "Unknown command: " << input << std::endl;
}