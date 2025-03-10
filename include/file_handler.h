#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <string>
#include <unordered_map>
#include "http.h"

class FileHandler {
public:
    explicit FileHandler(const std::string& publicDir);

    // Handle a file request, returns an HttpResponse
    HttpResponse handleRequest(const HttpRequest& request) const;

private:
    std::string publicDir;
    static const std::unordered_map<std::string, std::string> mimeTypes;

    // Helper methods
    std::string resolvePath(const std::string& requestPath) const;
    std::string getMimeType(const std::string& path) const;
    bool isPathSafe(const std::string& path) const;
    std::string readFile(const std::string& path) const;
};

#endif // FILE_HANDLER_H 