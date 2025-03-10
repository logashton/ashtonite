#include "file_handler.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

// Initialize MIME types
const std::unordered_map<std::string, std::string> FileHandler::mimeTypes = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".ico", "image/x-icon"},
    {".txt", "text/plain"},
    {".pdf", "application/pdf"},
    {".xml", "application/xml"},
    {".zip", "application/zip"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".ttf", "font/ttf"},
    {".eot", "application/vnd.ms-fontobject"}
};

FileHandler::FileHandler(const std::string& publicDir)
    : publicDir(publicDir) {
    if (!fs::exists(publicDir)) {
        fs::create_directories(publicDir);
        // Create a default index.html if it doesn't exist
        std::string indexPath = (fs::path(publicDir) / "index.html").string();
        if (!fs::exists(indexPath)) {
            std::ofstream index(indexPath);
            index << "<!DOCTYPE html>\n"
                  << "<html>\n"
                  << "<head>\n"
                  << "    <title>Welcome to Ashtonite</title>\n"
                  << "    <style>\n"
                  << "        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }\n"
                  << "        h1 { color: #333; }\n"
                  << "    </style>\n"
                  << "</head>\n"
                  << "<body>\n"
                  << "    <h1>Welcome to Ashtonite</h1>\n"
                  << "    <p>This is a lightweight, multi-threaded web server for static websites.</p>\n"
                  << "    <p>Place your files in the <code>public</code> directory to serve them.</p>\n"
                  << "</body>\n"
                  << "</html>";
        }
    }
}

HttpResponse FileHandler::handleRequest(const HttpRequest& request) const {
    if (request.method != "GET" && request.method != "HEAD") {
        throw HttpError("405 Method Not Allowed", "Only GET and HEAD methods are supported");
    }

    // Handle root path or empty path
    std::string requestPath = request.path;
    if (requestPath == "/" || requestPath.empty()) {
        requestPath = "/index.html";
    }

    std::string path = resolvePath(requestPath);
    
    if (!isPathSafe(path)) {
        throw HttpError("403 Forbidden", "Access denied");
    }

    if (!fs::exists(path)) {
        // Try index.html for directory requests
        if (fs::exists(path + "/index.html")) {
            path += "/index.html";
        } else {
            throw HttpError("404 Not Found", "File not found");
        }
    }

    if (fs::is_directory(path)) {
        path = (fs::path(path) / "index.html").string();
        if (!fs::exists(path)) {
            throw HttpError("404 Not Found", "Directory index not found");
        }
    }

    HttpResponse response;
    response.setContentType(getMimeType(path));

    if (request.method == "GET") {
        response.setBody(readFile(path));
    } else {
        // For HEAD requests, we only set Content-Length
        response.setHeader("Content-Length", std::to_string(fs::file_size(path)));
    }

    return response;
}

std::string FileHandler::resolvePath(const std::string& requestPath) const {
    // Normalize the path by removing duplicate slashes and resolving . and ..
    fs::path normalizedPath = fs::path(requestPath).lexically_normal();
    
    // Convert to the full filesystem path
    fs::path fullPath = fs::absolute(fs::path(publicDir) / normalizedPath.relative_path());
    
    return fullPath.string();
}

bool FileHandler::isPathSafe(const std::string& path) const {
    fs::path requestPath = fs::absolute(path);
    fs::path publicPath = fs::absolute(publicDir);
    
    // Try to create a relative path from the public directory to the requested path
    std::error_code ec;
    fs::path relativePath = fs::relative(requestPath, publicPath, ec);
    
    // If we can't create a relative path or there was an error, the path is not safe
    if (ec) {
        return false;
    }
    
    // Check if the relative path starts with ".." which would indicate going up directories
    std::string relativeStr = relativePath.string();
    if (relativeStr.empty() || relativeStr.find("..") != std::string::npos) {
        return false;
    }
    
    // Convert both paths to strings for prefix comparison
    std::string requestStr = requestPath.string();
    std::string publicStr = publicPath.string();
    
    // Ensure the request path starts with the public directory path
    return requestStr.substr(0, publicStr.length()) == publicStr;
}

std::string FileHandler::getMimeType(const std::string& path) const {
    fs::path filePath(path);
    std::string ext = filePath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    auto it = mimeTypes.find(ext);
    return it != mimeTypes.end() ? it->second : "application/octet-stream";
}

std::string FileHandler::readFile(const std::string& path) const {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw HttpError("500 Internal Server Error", "Failed to read file");
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
} 