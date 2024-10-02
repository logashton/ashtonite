#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <string>

class Server {
private:
    int serverSocket;
    sockaddr_in serverAddress;
    int port;
    std::string address;
    int clientSocket;

public:
    Server(int port, const std::string& address);
    ~Server();

    void start();
    int acceptConnection();
    std::string receiveMessage(int clientSocket);
    void close(int clientSocket);
    void closeClientConnection(int clientSocket);
    void close();
};

#endif // SERVER_H