#include "Parser/ConfigTokenizer.hpp"
#include <cctype>
#include <fstream>

std::string ConfigTokenizer::get_word(std::string::iterator& it, const std::string::iterator& end) {
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

void ConfigTokenizer::tokenize(const std::string& filename) {
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

void ConfigTokenizer::advance() {
    if (current_token_idx < tokens.size()) {
        m_current_token = tokens[current_token_idx++];
    } else {
        m_current_token = "";
    }
}

bool ConfigTokenizer::match(const std::string& expected) {
    if (m_current_token == expected) {
        advance();
        return true;
    }
    return false;
}

void ConfigTokenizer::expect(const std::string& expected) {
    if (m_current_token == expected) {
        advance();
    } else {
        throw std::runtime_error("Expected token: '" + expected + "', but got: '" + m_current_token + "'");
    }
}

std::string ConfigTokenizer::peek(size_t offset) const {
    size_t target_idx = current_token_idx > 0 ? current_token_idx - 1 + offset : offset;
    if (target_idx < tokens.size()) {
        return tokens[target_idx];
    }
    return "";
}

std::string ConfigTokenizer::current_token() const {
    return m_current_token;
}

bool ConfigTokenizer::empty() const {
    return m_current_token.empty();
}

ConfigTokenizer::ConfigTokenizer(const std::string& filename) : current_token_idx(0) {
    tokenize(filename);
    advance();
}

ConfigTokenizer::~ConfigTokenizer() {}
