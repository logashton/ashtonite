#include "server.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>

Server::Server(const Config& config)
    : config(config)
    , serverSocket(-1)
    , threadPool(std::make_unique<ThreadPool>(config.getThreadPoolSize()))
    , fileHandler(std::make_unique<FileHandler>(config.getPublicDir()))
    , running(false)
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to set socket options");
    }

    // Set non-blocking mode
    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("Failed to get socket flags");
    }
    if (fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set non-blocking mode");
    }
}

Server::~Server() {
    stop();
}

void Server::start() {
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(config.getPort());
    serverAddress.sin_addr.s_addr = inet_addr(config.getAddress().c_str());

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, config.getMaxConnections()) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    running = true;
    std::cout << "Server listening on " << config.getAddress() << ":" << config.getPort() << std::endl;

    while (running) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // No pending connections, sleep for a short time
                usleep(1000);
                continue;
            }
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            continue;
        }

        // Set socket timeout
        struct timeval tv;
        tv.tv_sec = SOCKET_TIMEOUT_SECONDS;
        tv.tv_usec = 0;
        if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            std::cerr << "Failed to set receive timeout" << std::endl;
            close(clientSocket);
            continue;
        }
        if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
            std::cerr << "Failed to set send timeout" << std::endl;
            close(clientSocket);
            continue;
        }

        threadPool->enqueue([this, clientSocket] {
            handleClient(clientSocket);
        });
    }
}

void Server::stop() {
    running = false;
    if (serverSocket != -1) {
        close(serverSocket);
        serverSocket = -1;
    }
}

void Server::handleClient(int clientSocket) {
    try {
        bool keepAlive = true;
        while (keepAlive && running) {
            char buffer[BUFFER_SIZE];
            ssize_t bytesRead;
            std::string request;
            bool requestComplete = false;

            // Set a timeout for receiving the next request
            struct timeval tv;
            tv.tv_sec = SOCKET_TIMEOUT_SECONDS;
            tv.tv_usec = 0;
            setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

            // Read the request
            while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
                buffer[bytesRead] = '\0';
                request.append(buffer);

                // Check if we've received the complete request
                if (request.find("\r\n\r\n") != std::string::npos) {
                    requestComplete = true;
                    break;
                }
            }

            // Handle connection closure or timeout
            if (bytesRead <= 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Timeout occurred
                    std::cout << "Connection timed out" << std::endl;
                }
                break;
            }

            if (!requestComplete) {
                break;
            }

            auto httpRequest = HttpRequest::parse(request);
            keepAlive = httpRequest.isKeepAlive();

            try {
                HttpResponse response = fileHandler->handleRequest(httpRequest);
                
                // Set keep-alive header based on the request
                if (keepAlive) {
                    response.setHeader("Connection", "keep-alive");
                    response.setHeader("Keep-Alive", "timeout=" + std::to_string(SOCKET_TIMEOUT_SECONDS));
                } else {
                    response.setHeader("Connection", "close");
                }

                sendResponse(clientSocket, response);
            } catch (const HttpError& e) {
                handleError(clientSocket, e);
                break;
            } catch (const std::exception& e) {
                HttpError error("500 Internal Server Error", e.what());
                handleError(clientSocket, error);
                break;
            }

            // If not keep-alive, close the connection
            if (!keepAlive) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling client: " << e.what() << std::endl;
    }

    // Always close the socket when we're done
    close(clientSocket);
}

void Server::processRequest(int clientSocket, const HttpRequest& request) {
    try {
        HttpResponse response = fileHandler->handleRequest(request);
        
        if (request.isKeepAlive()) {
            response.setHeader("Connection", "keep-alive");
            response.setHeader("Keep-Alive", "timeout=" + std::to_string(SOCKET_TIMEOUT_SECONDS));
        }

        sendResponse(clientSocket, response);

        if (!request.isKeepAlive()) {
            close(clientSocket);
        }
    } catch (const HttpError& e) {
        handleError(clientSocket, e);
    } catch (const std::exception& e) {
        HttpError error("500 Internal Server Error", e.what());
        handleError(clientSocket, error);
    }
}

void Server::sendResponse(int clientSocket, const HttpResponse& response) {
    std::string responseStr = response.toString();
    ssize_t totalSent = 0;
    size_t remaining = responseStr.length();

    while (remaining > 0) {
        ssize_t sent = send(clientSocket, responseStr.c_str() + totalSent, remaining, 0);
        if (sent <= 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("Failed to send response");
        }
        totalSent += sent;
        remaining -= sent;
    }
}

void Server::handleError(int clientSocket, const HttpError& error) {
    HttpResponse response(error.getStatus());
    response.setContentType("text/html");
    response.setBody("<html><body><h1>" + error.getStatus() + "</h1><p>" + error.what() + "</p></body></html>");
    
    try {
        sendResponse(clientSocket, response);
    } catch (const std::exception& e) {
        std::cerr << "Error sending error response: " << e.what() << std::endl;
    }
    
    close(clientSocket);
}