#include "Parser/ConfigParser.hpp"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

std::string get_word(std::string::iterator& it, const std::string::iterator& end) {
    while (it != end && std::isspace(*it))
        it++;
    if (it == end)
        return "";

    if (*it == '{' || *it == '}' || *it == ':' || *it == ';')
        return std::string(1, *(it++));

    std::string::iterator start = it;
    while (it != end && !std::isspace(*it) && *it != '{' && *it != '}' && *it != ':' && *it != ';') {
        ++it;
    }
    return std::string(start, it);
}

void Parser::tokenize(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file");
    }

    std::string buffer;
    while (std::getline(file, buffer)) {
        size_t comment_pos = buffer.find('#');

        if (comment_pos != std::string::npos)
            buffer.erase(comment_pos);

        std::string::iterator wrd_start = buffer.begin();

        while (wrd_start != buffer.end()) {
            std::string word = get_word(wrd_start, buffer.end());
            if (!word.empty())
                tokens.push_back(word);
        }
    }
}

void Parser::advance() {
    if (current_token_idx < tokens.size()) {
        current_token = tokens[current_token_idx++];
    } else {
        current_token = "";
    }
}

bool Parser::match(const std::string& expected) {
    if (current_token == expected) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(const std::string& expected) {
    if (current_token == expected) {
        advance();
    } else {
        throw std::runtime_error("Expected token: '" + expected + "', but got: '" + current_token + "'");
    }
}

std::string Parser::peek(size_t offset) {
    size_t target_idx = current_token_idx > 0 ? current_token_idx - 1 + offset : offset;
    if (target_idx < tokens.size()) {
        return tokens[target_idx];
    }
    return "";
}

Parser::Parser(const std::string& filename) : current_token_idx(0) {
    tokenize(filename);
    advance();
}

Parser::~Parser() {}

Config::~Config() {
    for (size_t i = 0; i < all_blocks.size(); ++i) {
        delete all_blocks[i];
    }
}

LocationConfig Parser::parse_location_block(const std::string& path) {
    LocationConfig loc;
    loc.base_path = path;
    
    while (!current_token.empty() && current_token != "}") {
        if (match("alias")) {
            loc.alias = current_token;
            advance();
            expect(";");
        } else if (match("allow_methods")) {
            loc.allow_methods = 0;
            while (current_token != ";") {
                if (current_token == "GET") loc.allow_methods |= 1;
                else if (current_token == "POST") loc.allow_methods |= 2;
                else if (current_token == "DELETE") loc.allow_methods |= 4;
                advance();
            }
            expect(";");
        } else if (match("autoindex")) {
            loc.directory_listing = (current_token == "on");
            advance();
            expect(";");
        } else if (match("index")) {
            loc.index_file = current_token;
            advance();
            expect(";");
        } else if (match("return")) {
            int code = std::atoi(current_token.c_str());
            advance();
            std::string redirect_url = current_token;
            advance();
            expect(";");
            loc.redirect = std::make_pair(code, redirect_url);
        } else if (match("upload_enable")) {
            loc.upload_enabled = (current_token == "on");
            advance();
            expect(";");
        } else if (match("upload_store")) {
            loc.upload_path = current_token;
            advance();
            expect(";");
        } else if (match("cgi_pass")) {
            std::string ext = current_token;
            advance();
            std::string cgi_path = current_token;
            advance();
            expect(";");
            loc.cgi[ext] = cgi_path;
        } else {
            throw std::runtime_error("Unknown location directive: " + current_token);
        }
    }
    expect("}");
    return loc;
}

void Parser::parse_server_block(Config* config) {
    expect("{");
    ServerBlock* server = new ServerBlock();
    config->all_blocks.push_back(server);
    
    std::vector<ListenAddress> listens;
    std::vector<std::string> server_names;
    
    while (!current_token.empty() && current_token != "}") {
        if (match("listen")) {
            std::string host_port = current_token;
            advance();
            expect(";");
            
            ListenAddress addr;
            size_t colon = host_port.find(':');
            if (colon != std::string::npos) {
                addr.host = host_port.substr(0, colon);
                addr.port = std::atoi(host_port.substr(colon + 1).c_str());
            } else {
                bool is_port = true;
                for (size_t i = 0; i < host_port.size(); ++i) {
                    if (!isdigit(host_port[i])) {
                        is_port = false;
                        break;
                    }
                }
                if (is_port) {
                    addr.host = "0.0.0.0";
                    addr.port = std::atoi(host_port.c_str());
                } else {
                    addr.host = host_port;
                    addr.port = 80;
                }
            }
            listens.push_back(addr);
        } else if (match("server_name")) {
            while (current_token != ";") {
                server_names.push_back(current_token);
                advance();
            }
            expect(";");
        } else if (match("client_max_body_size")) {
            server->client_max_body_size = std::atoi(current_token.c_str());
            advance();
            expect(";");
        } else if (match("error_page")) {
            int code = std::atoi(current_token.c_str());
            advance();
            std::string page = current_token;
            advance();
            expect(";");
            server->error_pages[code] = page;
        } else if (match("location")) {
            std::string path = current_token;
            advance();
            expect("{");
            server->locations[path] = parse_location_block(path);
        } else {
            throw std::runtime_error("Unknown server directive: " + current_token);
        }
    }
    expect("}");
    
    if (listens.empty()) {
        ListenAddress default_addr;
        default_addr.host = "0.0.0.0";
        default_addr.port = 80;
        listens.push_back(default_addr);
    }
    if (server_names.empty()) {
        server_names.push_back("");
    }
    
    for (size_t i = 0; i < listens.size(); ++i) {
        for (size_t j = 0; j < server_names.size(); ++j) {
            config->routing_table[listens[i]][server_names[j]] = server;
        }
    }
}

Config* Parser::parse() {
    Config* config = new Config();
    
    while (!current_token.empty()) {
        if (match("server")) {
            parse_server_block(config);
        } else {
            throw std::runtime_error("Expected 'server' block, got: " + current_token);
        }
    }
    
    return config;
}
