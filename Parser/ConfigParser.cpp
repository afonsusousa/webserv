#include "Parser/ConfigParser.hpp"
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <cctype>

ConfigParser::ConfigParser(const std::string& filename) : tokenizer(filename) {}

ConfigParser::~ConfigParser() {}

bool ConfigParser::is_number(const std::string& str) {
    if (str.empty()) return false;
    for (size_t i = 0; i < str.length(); ++i) {
        if (!std::isdigit(str[i])) return false;
    }
    return true;
}

LocationConfig ConfigParser::parse_location_block(const std::string& path) {
    LocationConfig loc;
    loc.base_path = path;
    
    while (!tokenizer.empty() && tokenizer.current_token() != "}") {
        if (tokenizer.match("alias")) {
            loc.alias = tokenizer.current_token();
            tokenizer.advance();
            tokenizer.expect(";");
        } else if (tokenizer.match("allow_methods")) {
            loc.allow_methods = 0;
            while (!tokenizer.empty() && tokenizer.current_token() != ";") {
                if (tokenizer.current_token() == "GET") loc.allow_methods |= 1;
                else if (tokenizer.current_token() == "POST") loc.allow_methods |= 2;
                else if (tokenizer.current_token() == "DELETE") loc.allow_methods |= 4;
                else tokenizer.throw_error("Unknown HTTP method in allow_methods: '" + tokenizer.current_token() + "'");
                tokenizer.advance();
            }
            tokenizer.expect(";");
        } else if (tokenizer.match("autoindex")) {
            std::string val = tokenizer.current_token();
            if (val == "on") loc.directory_listing = true;
            else if (val == "off") loc.directory_listing = false;
            else tokenizer.throw_error("Invalid autoindex value (must be 'on' or 'off'): '" + val + "'");
            tokenizer.advance();
            tokenizer.expect(";");
        } else if (tokenizer.match("index")) {
            loc.index_file = tokenizer.current_token();
            tokenizer.advance();
            tokenizer.expect(";");
        } else if (tokenizer.match("return")) {
            if (!is_number(tokenizer.current_token())) {
                tokenizer.throw_error("Invalid return status code: '" + tokenizer.current_token() + "'");
            }
            int code = std::atoi(tokenizer.current_token().c_str());
            tokenizer.advance();
            std::string redirect_url = tokenizer.current_token();
            tokenizer.advance();
            tokenizer.expect(";");
            loc.redirect = std::make_pair(code, redirect_url);
        } else if (tokenizer.match("upload_enable")) {
            std::string val = tokenizer.current_token();
            if (val == "on") loc.upload_enabled = true;
            else if (val == "off") loc.upload_enabled = false;
            else tokenizer.throw_error("Invalid upload_enable value (must be 'on' or 'off'): '" + val + "'");
            tokenizer.advance();
            tokenizer.expect(";");
        } else if (tokenizer.match("upload_store")) {
            loc.upload_path = tokenizer.current_token();
            tokenizer.advance();
            tokenizer.expect(";");
        } else if (tokenizer.match("cgi_pass")) {
            std::string ext = tokenizer.current_token();
            tokenizer.advance();
            std::string cgi_path = tokenizer.current_token();
            tokenizer.advance();
            tokenizer.expect(";");
            loc.cgi[ext] = cgi_path;
        } else {
            tokenizer.throw_error("Unknown location directive: '" + tokenizer.current_token() + "'");
        }
    }
    tokenizer.expect("}");
    return loc;
}

void ConfigParser::parse_server_block(Config* config) {
    tokenizer.expect("{");
    ServerBlock* server = new ServerBlock();
    config->all_blocks.push_back(server);
    
    std::vector<ListenAddress> listens;
    std::vector<std::string> server_names;
    
    while (!tokenizer.empty() && tokenizer.current_token() != "}") {
        if (tokenizer.match("listen")) {
            std::string first_token = tokenizer.current_token();
            tokenizer.advance();
            
            ListenAddress addr;
            
            if (tokenizer.match(":")) { // format is host:port
                addr.host = first_token;
                if (!is_number(tokenizer.current_token())) {
                     tokenizer.throw_error("Invalid port in listen directive: '" + tokenizer.current_token() + "'");
                }
                addr.port = std::atoi(tokenizer.current_token().c_str());
                tokenizer.advance();
            } else {
                if (is_number(first_token)) {
                    addr.host = "0.0.0.0";
                    addr.port = std::atoi(first_token.c_str());
                } else {
                    addr.host = first_token;
                    addr.port = 80;
                }
            }
            tokenizer.expect(";");
            listens.push_back(addr);
        } else if (tokenizer.match("server_name")) {
            while (!tokenizer.empty() && tokenizer.current_token() != ";") {
                if (tokenizer.current_token() == "{" || tokenizer.current_token() == "}") {
                    tokenizer.throw_error("Unexpected block bound inside server_name");
                }
                server_names.push_back(tokenizer.current_token());
                tokenizer.advance();
            }
            tokenizer.expect(";");
        } else if (tokenizer.match("client_max_body_size")) {
            if (!is_number(tokenizer.current_token())) {
                tokenizer.throw_error("Invalid client_max_body_size value: '" + tokenizer.current_token() + "'");
            }
            server->client_max_body_size = std::atoi(tokenizer.current_token().c_str());
            tokenizer.advance();
            tokenizer.expect(";");
        } else if (tokenizer.match("error_page")) {
            if (!is_number(tokenizer.current_token())) {
                tokenizer.throw_error("Invalid status code for error_page: '" + tokenizer.current_token() + "'");
            }
            int code = std::atoi(tokenizer.current_token().c_str());
            tokenizer.advance();
            std::string page = tokenizer.current_token();
            tokenizer.advance();
            tokenizer.expect(";");
            server->error_pages[code] = page;
        } else if (tokenizer.match("location")) {
            std::string path = tokenizer.current_token();
            tokenizer.advance();
            tokenizer.expect("{");
            server->locations[path] = parse_location_block(path);
        } else {
            tokenizer.throw_error("Unknown server directive: '" + tokenizer.current_token() + "'");
        }
    }
    tokenizer.expect("}");
    
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

Config* ConfigParser::parse() {
    Config* config = new Config();
    
    while (!tokenizer.empty()) {
        if (tokenizer.match("server")) {
            parse_server_block(config);
        } else {
            tokenizer.throw_error("Expected 'server' block, got: '" + tokenizer.current_token() + "'");
        }
    }
    
    return config;
}
