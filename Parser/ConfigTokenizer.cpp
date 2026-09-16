#include "Parser/ConfigTokenizer.hpp"
#include <cctype>
#include <sstream>

ConfigTokenizer::ConfigTokenizer(const std::string& filename)
    : line_pos(0), line_number(0), col_number(0) {
    file.open(filename.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filename);
    }
    advance();
}

ConfigTokenizer::~ConfigTokenizer() {
    if (file.is_open()) {
        file.close();
    }
}

std::string ConfigTokenizer::get_word() {
    while (true) {
        while (line_pos >= current_line.length()) {
            if (!std::getline(file, current_line)) {
                return "";
            }
            line_pos = 0;
            line_number++;

            size_t comment_pos = current_line.find('#');
            if (comment_pos != std::string::npos) {
                current_line.erase(comment_pos);
            }
        }

        while (line_pos < current_line.length() && std::isspace(current_line[line_pos])) {
            line_pos++;
        }

        if (line_pos < current_line.length()) {
            break;
        }
    }

    col_number = line_pos + 1;

    char c = current_line[line_pos];
    if (c == '{' || c == '}' || c == ':' || c == ';') {
        line_pos++;
        return std::string(1, c);
    }

    size_t start = line_pos;

    while (line_pos < current_line.length()) {
        c = current_line[line_pos];
        if (std::isspace(c) || c == '{' || c == '}' || c == ':' || c == ';') {
            break;
        }
        line_pos++;
    }

    return current_line.substr(start, line_pos - start);
}

void ConfigTokenizer::advance() {
    m_current_token = get_word();
}

bool ConfigTokenizer::match(const std::string& expected) {
    if (m_current_token == expected) {
        advance();
        return true;
    }
    return false;
}

void ConfigTokenizer::throw_error(const std::string& message) const {
    std::stringstream ss;
    ss << message << " at line " << line_number << ", col " << col_number << "\n";
    ss << current_line << "\n";

    for (size_t i = 0; i < current_line.length() && i < col_number - 1; ++i) {
        if (current_line[i] == '\t') ss << '\t';
        else ss << ' ';
    }
    ss << "\033[1;31m^\033[0m\n"; // ^

    throw std::runtime_error(ss.str());
}

void ConfigTokenizer::expect(const std::string& expected) {
    if (m_current_token == expected) {
        advance();
    } else {
        throw_error("Expected token: '" + expected + "', but got: '" + m_current_token + "'");
    }
}

std::string ConfigTokenizer::current_token() const {
    return m_current_token;
}

bool ConfigTokenizer::empty() const {
    return m_current_token.empty();
}

size_t ConfigTokenizer::get_line_num() const {
    return line_number;
}

size_t ConfigTokenizer::get_col_num() const {
    return col_number;
}
