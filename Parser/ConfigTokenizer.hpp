#pragma once

#include <string>
#include <fstream>
#include <stdexcept>

class ConfigTokenizer {
public:
    ConfigTokenizer(const std::string& filename);
    ~ConfigTokenizer();

    void advance();
    bool match(const std::string& expected);
    void expect(const std::string& expected);
    
    std::string current_token() const;
    bool empty() const;

    size_t get_line_num() const;
    size_t get_col_num() const;

    void throw_error(const std::string& message) const;

private:
    std::ifstream file;
    std::string   current_line;
    size_t        line_pos;
    size_t        line_number;
    size_t        col_number;
    std::string   m_current_token;

    std::string get_word();
};
