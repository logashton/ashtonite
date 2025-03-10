#include "server.h"
#include "config.h"
#include <iostream>
#include <csignal>
#include <cstdlib>

std::unique_ptr<Server> server;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(signum);
}

int main(int argc, char* argv[]) {
    try {
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        // Default configuration
        std::string address = "0.0.0.0";  // Listen on all interfaces by default
        uint16_t port = 8080;
        std::string publicDir = "public";
        size_t threadPoolSize = std::thread::hardware_concurrency();
        size_t maxConnections = 1000;

        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--address" && i + 1 < argc) {
                address = argv[++i];
            } else if (arg == "--port" && i + 1 < argc) {
                port = static_cast<uint16_t>(std::stoi(argv[++i]));
            } else if (arg == "--public-dir" && i + 1 < argc) {
                publicDir = argv[++i];
            } else if (arg == "--threads" && i + 1 < argc) {
                threadPoolSize = std::stoul(argv[++i]);
            } else if (arg == "--max-connections" && i + 1 < argc) {
                maxConnections = std::stoul(argv[++i]);
            } else if (arg == "--help") {
                std::cout << "Usage: " << argv[0] << " [options]\n"
                         << "Options:\n"
                         << "  --address ADDR       Address to bind to (default: 0.0.0.0)\n"
                         << "  --port PORT         Port to listen on (default: 8080)\n"
                         << "  --public-dir DIR    Directory to serve files from (default: public)\n"
                         << "  --threads N         Number of worker threads (default: CPU cores)\n"
                         << "  --max-connections N Maximum number of pending connections (default: 1000)\n"
                         << "  --help             Show this help message\n";
                return 0;
            }
        }

        Config config(address, port, publicDir, threadPoolSize, maxConnections);
        server = std::make_unique<Server>(config);
        
        std::cout << "Starting server with configuration:\n"
                 << "  Address: " << address << "\n"
                 << "  Port: " << port << "\n"
                 << "  Public directory: " << publicDir << "\n"
                 << "  Worker threads: " << threadPoolSize << "\n"
                 << "  Max connections: " << maxConnections << "\n";

        server->start();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


