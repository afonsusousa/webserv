#pragma once

#include <stdint.h>
#include <string>
#include <map>
#include <vector>

struct ListenAddress {
    std::string host;
    uint16_t    port;

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
};

class ServerBlock {
public:
    size_t                                client_max_body_size;
    
    std::map<int, std::string>            error_pages;
    std::map<std::string, LocationConfig> locations;
};

class Config {
public:
    std::vector<ServerBlock*>                                     all_blocks;
    std::map<ListenAddress, std::map<std::string, ServerBlock*> > routing_table;

    ~Config();
};