#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstdint>

class Config {
public:
    Config(const std::string& address = "127.0.0.1",
           uint16_t port = 8080,
           const std::string& publicDir = "public",
           size_t threadPoolSize = 4,
           size_t maxConnections = 1000);

    const std::string& getAddress() const { return address; }
    uint16_t getPort() const { return port; }
    const std::string& getPublicDir() const { return publicDir; }
    size_t getThreadPoolSize() const { return threadPoolSize; }
    size_t getMaxConnections() const { return maxConnections; }

private:
    std::string address;
    uint16_t port;
    std::string publicDir;
    size_t threadPoolSize;
    size_t maxConnections;
};

#endif // CONFIG_H 