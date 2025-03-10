#include "config.h"

Config::Config(const std::string& address,
               uint16_t port,
               const std::string& publicDir,
               size_t threadPoolSize,
               size_t maxConnections)
    : address(address)
    , port(port)
    , publicDir(publicDir)
    , threadPoolSize(threadPoolSize)
    , maxConnections(maxConnections)
{} 