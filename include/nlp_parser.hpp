#pragma once
#include <string>
#include <map>
#include <functional>
#include "obs_controller.hpp"

class NLPParser {
public:
    NLPParser(OBSController& controller);
    void processCommand(const std::string& input);

private:
    OBSController& controller_;
    std::map<std::string, std::function<void(std::string)>> command_map_;
};