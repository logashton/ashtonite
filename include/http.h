#ifndef HTTP_H
#define HTTP_H

#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

class HttpRequest {
public:
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    static HttpRequest parse(const std::string& raw);
    bool isKeepAlive() const;
};

class HttpResponse {
public:
    HttpResponse(const std::string& status = "200 OK");

    void setStatus(const std::string& status);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    void setContentType(const std::string& contentType);

    std::string toString() const;

private:
    std::string status;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpError : public std::runtime_error {
public:
    HttpError(const std::string& status, const std::string& message)
        : std::runtime_error(message), status(status) {}
    
    const std::string& getStatus() const { return status; }

private:
    std::string status;
};

#endif // HTTP_H 