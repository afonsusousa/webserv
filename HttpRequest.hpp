#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>

class HttpRequest {
public:
    enum State {
        REQUEST_LINE,
        HEADERS,
        BODY,
        COMPLETE,
        ERROR
    };

    State                               state;
    std::string                         method;
    std::string                         uri;
    std::string                         version;
    std::map<std::string, std::string>  headers;
    std::string                         body;
    size_t                              content_length;

    HttpRequest();
    ~HttpRequest();

    void reset();
    bool parse(std::string& raw_data);

private:
    bool parse_request_line(std::string& raw_data);
    bool parse_headers(std::string& raw_data);
    bool parse_body(std::string& raw_data);
    std::string trim(const std::string& str);
    bool get_line(std::string& raw_data, std::string& line);
};

#endif
