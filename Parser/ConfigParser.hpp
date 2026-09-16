#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include <vector>

struct ListenAddress {
    std::string host;
    uint16_t    port;

    ListenAddress() : host(""), port(0) {}

    bool operator<(const ListenAddress& other) const {
        if (host != other.host)
            return host < other.host;
        return port < other.port;
    }
};

class LocationConfig {
public:
    std::string                        base_path;
    std::string                        alias;
    uint16_t                           allow_methods;
    bool                               directory_listing;
    std::string                        index_file;

    std::pair<int, std::string>        redirect;
    bool                               upload_enabled;
    std::string                        upload_path;
    std::map<std::string, std::string> cgi;

    LocationConfig()
        : base_path(""), alias(""), allow_methods(7), directory_listing(false),
          index_file(""), redirect(std::make_pair(0, "")), upload_enabled(false),
          upload_path("") {}
};

class ServerBlock {
public:
    size_t                                client_max_body_size;
    
    std::map<int, std::string>            error_pages;
    std::map<std::string, LocationConfig> locations;

    ServerBlock() : client_max_body_size(1048576) {}
};

class Config {
public:
    std::vector<ServerBlock*>                                     all_blocks;
    std::map<ListenAddress, std::map<std::string, ServerBlock*> > routing_table;

    ~Config();
};

class Parser {
public:
    Parser(const std::string& filename);
    ~Parser();

    Config* parse();

private:
    std::vector<std::string> tokens;
    size_t current_token_idx;
    std::string current_token;

    void advance();
    bool match(const std::string& expected);
    void expect(const std::string& expected);
    void tokenize(const std::string& filename);
    std::string peek(size_t offset = 0);
    
    void parse_server_block(Config* config);
    LocationConfig parse_location_block(const std::string& path);

};