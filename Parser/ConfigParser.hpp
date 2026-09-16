#pragma once

#include "Config.hpp"
#include "Parser/ConfigTokenizer.hpp"
#include <string>

class ConfigParser {
public:
    ConfigParser(const std::string& filename);
    ~ConfigParser();

    Config* parse();

private:
    ConfigTokenizer tokenizer;
    
    void parse_server_block(Config* config);
    LocationConfig parse_location_block(const std::string& path);
    bool is_number(const std::string& str);
};
