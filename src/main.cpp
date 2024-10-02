#include "server.h"
#include <iostream>
#include <thread>
#include <vector>

void handleClient(Server& server, int clientSocket) {
    std::string message;
    while (true) {
        message = server.receiveMessage(clientSocket);
        
        if (message.size() <= 0) {
            break;
        }

        std::cout << "Message from client " << clientSocket << ": " << message << std::endl;
    }
    std::cout << "Client " << clientSocket << " disconnected" << std::endl;
    server.closeClientConnection(clientSocket);
}

int main()
{
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


