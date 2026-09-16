#pragma once

#include <string>
#include <vector>
#include <stdexcept>

class ConfigTokenizer {
public:
    ConfigTokenizer(const std::string& filename);
    ~ConfigTokenizer();

    void advance();
    bool match(const std::string& expected);
    void expect(const std::string& expected);
    std::string peek(size_t offset = 0) const;
    
    std::string current_token() const;
    bool empty() const;

private:
    std::vector<std::string> tokens;
    size_t current_token_idx;
    std::string m_current_token;

    void tokenize(const std::string& filename);
    std::string get_word(std::string::iterator& it, const std::string::iterator& end);
};
