#include "server.h"
#include "parse.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string& address) : port(port), address(address) {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        throw std::runtime_error("Failed to create socket");
    }
}

Server::~Server() {
    close();
}

void Server::start() {
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(address.c_str());

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(serverSocket, 5) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "Server listening on " << address << ":" << port << std::endl;
}

int Server::acceptConnection() {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        throw std::runtime_error("Failed to accept connection");
    }
    std::cout << "Client connected" << std::endl;
    return clientSocket;
}

std::string Server::receiveMessage(int clientSocket) {
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    
    if (bytesReceived <= 0) {
        return "";
    }

    std::string headers(buffer, bytesReceived);
    std::unordered_map<std::string, std::string> headerMap = parseHeaders(headers);

    return std::string(buffer, bytesReceived);
}

void Server::closeClientConnection(int clientSocket) {
    if (clientSocket != -1) {
        ::close(clientSocket);
    }
}

void Server::close() {
    if (serverSocket != -1) {
        ::close(serverSocket);
        serverSocket = -1;
    }
}

void Server::sendHttpResponse(int clientSocket, const std::string& status, const std::string& contentType, const std::string& body) {
    std::string response = "HTTP/1.1 " + status + "\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
    response += "\r\n";
    response += body;

    send(clientSocket, response.c_str(), response.length(), 0);
}