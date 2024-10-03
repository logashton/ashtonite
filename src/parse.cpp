#include <iostream>
#include <string>
#include <unordered_map>
#include "parse.h"
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <cstddef>

std::unordered_map<std::string, std::string> parseHeaders(const std::string& headers) {
    std::unordered_map<std::string, std::string> headerMap;
    std::istringstream stream(headers);
    std::string line;

    // Parsing first line by itself to get the method, path, and HTTP version
    if (std::getline(stream, line)) {
        std::istringstream requestLineStream(line);
        std::string method, path, httpVersion;
        requestLineStream >> method >> path >> httpVersion;
        headerMap["Method"] = method;
        headerMap["Path"] = path;
        headerMap["HTTP-Version"] = httpVersion;
    }

    while (std::getline(stream, line)) {
        if (line.empty() || line == "\r") break;
        
        int colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);
            
            headerMap[key] = value;
        }
    }

    return headerMap;
}



