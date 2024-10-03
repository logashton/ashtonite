#include "server.h"
#include <iostream>
#include <thread>
#include <vector>

void handleClient(Server& server, int clientSocket) {
    std::string request = server.receiveMessage(clientSocket);
    
    if (request.empty()) {
        std::cout << "Client " << clientSocket << " disconnected" << std::endl;
        server.closeClientConnection(clientSocket);
        return;
    }

    std::cout << "Request from client " << clientSocket << ":\n" << request << std::endl;

    // Just for testing
    std::string notFoundBody = "<html><body><h1>404 Not Found</h1></body></html>";
    server.sendHttpResponse(clientSocket, "404 Not Found", "text/html", notFoundBody);

    server.closeClientConnection(clientSocket);
}

int main() {
    Server server(8080, "127.0.0.1");
    std::vector<std::thread> clientThreads;
    bool serverRunning = true;
    
    server.start();

    while (serverRunning) {
        int clientSocket = server.acceptConnection();
        if (clientSocket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        clientThreads.emplace_back(handleClient, std::ref(server), clientSocket);
    }

    /*
    for (auto& thread : clientThreads) {
        thread.join();
    }
    */

    server.close();
    return 0;
}


