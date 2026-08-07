#include "HttpRequest.hpp"
#include <algorithm>

HttpRequest::HttpRequest() {
    reset();
}

HttpRequest::~HttpRequest() {}

void HttpRequest::reset() {
    state = REQUEST_LINE;
    method.clear();
    uri.clear();
    version.clear();
    headers.clear();
    body.clear();
    content_length = 0;
}

std::string HttpRequest::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

bool HttpRequest::get_line(std::string& raw_data, std::string& line) {
    size_t pos = raw_data.find("\r\n");
    if (pos == std::string::npos) {
        return false;
    }
    line = raw_data.substr(0, pos);
    raw_data.erase(0, pos + 2);
    return true;
}

bool HttpRequest::parse(std::string& raw_data) {
    while (!raw_data.empty() && state != COMPLETE && state != ERROR) {
        switch (state) {
            case REQUEST_LINE:
                if (!parse_request_line(raw_data)) return true; // Wait for more data
                break;
            case HEADERS:
                if (!parse_headers(raw_data)) return true; // Wait for more data
                break;
            case BODY:
                if (!parse_body(raw_data)) return true; // Wait for more data
                break;
            default:
                break;
        }
    }
    return true;
}

bool HttpRequest::parse_request_line(std::string& raw_data) {
    std::string line;
    if (!get_line(raw_data, line)) {
        return false; // Not enough data yet
    }

    std::istringstream iss(line);
    if (!(iss >> method >> uri >> version)) {
        state = ERROR;
        return false;
    }

    state = HEADERS;
    return true;
}

bool HttpRequest::parse_headers(std::string& raw_data) {
    std::string line;
    while (get_line(raw_data, line)) {
        if (line.empty()) {
            if (headers.count("Content-Length")) {
                std::istringstream iss(headers["Content-Length"]);
                iss >> content_length;
                state = BODY;
            } else {
                state = COMPLETE;
            }
            return true;
        }

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = trim(line.substr(0, colon_pos));
            std::string value = trim(line.substr(colon_pos + 1));
            headers[key] = value;
        } else {
            state = ERROR;
            return false;
        }
    }
    return false;
}

bool HttpRequest::parse_body(std::string& raw_data) {
    size_t bytes_to_read = content_length - body.size();
    if (raw_data.size() >= bytes_to_read) {
        body += raw_data.substr(0, bytes_to_read);
        raw_data.erase(0, bytes_to_read);
        state = COMPLETE;
        return true;
    } else {
        body += raw_data;
        raw_data.clear();
        return false; // Need more data
    }
}
