#include "http.h"
#include <sstream>
#include <algorithm>

HttpRequest HttpRequest::parse(const std::string& raw) {
    HttpRequest request;
    std::istringstream stream(raw);
    std::string line;

    // Parse request line
    if (std::getline(stream, line)) {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        std::istringstream requestLine(line);
        requestLine >> request.method >> request.path >> request.version;
    }

    // Parse headers
    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        auto colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            value.erase(0, value.find_first_not_of(" "));
            value.erase(value.find_last_not_of(" ") + 1);
            
            request.headers[key] = value;
        }
    }

    // Parse body if Content-Length is present
    auto it = request.headers.find("Content-Length");
    if (it != request.headers.end()) {
        size_t contentLength = std::stoul(it->second);
        std::string body;
        char ch;
        while (contentLength > 0 && stream.get(ch)) {
            body.push_back(ch);
            --contentLength;
        }
        request.body = body;
    }

    return request;
}

bool HttpRequest::isKeepAlive() const {
    auto it = headers.find("Connection");
    if (it != headers.end()) {
        return it->second == "keep-alive";
    }
    return version == "HTTP/1.1"; // HTTP/1.1 defaults to keep-alive
}

HttpResponse::HttpResponse(const std::string& status)
    : status(status) {
    headers["Server"] = "Ashtonite/1.0";
    headers["Connection"] = "close";
}

void HttpResponse::setStatus(const std::string& newStatus) {
    status = newStatus;
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}

void HttpResponse::setBody(const std::string& newBody) {
    body = newBody;
    headers["Content-Length"] = std::to_string(body.length());
}

void HttpResponse::setContentType(const std::string& contentType) {
    headers["Content-Type"] = contentType;
}

std::string HttpResponse::toString() const {
    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n";
    
    for (const auto& header : headers) {
        response << header.first << ": " << header.second << "\r\n";
    }
    
    response << "\r\n" << body;
    return response.str();
} 