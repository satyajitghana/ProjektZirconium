#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace zirconium {
auto parse_config(std::string config_file) {
    std::map<std::string, std::string> config;

    std::ifstream config_fp;
    config_fp.open(config_file);

    if (not config_fp) {
        throw std::runtime_error("error opening config file: " + config_file);
    }

    std::string line;
    while (std::getline(config_fp, line)) {
        std::string key = line.substr(0, line.find_first_of(":"));
        std::string value = line.substr(line.find_first_of(":") + 1, line.length());
        config[key] = value;
    }

    config_fp.close();

    return config;
}
}  // namespace zirconium