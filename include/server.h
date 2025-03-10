#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <string>
#include <memory>
#include "config.h"
#include "thread_pool.h"
#include "file_handler.h"
#include "http.h"

class Server {
public:
    explicit Server(const Config& config);
    ~Server();

    void start();
    void stop();

private:
    Config config;
    int serverSocket;
    sockaddr_in serverAddress;
    std::unique_ptr<ThreadPool> threadPool;
    std::unique_ptr<FileHandler> fileHandler;
    bool running;

    void handleClient(int clientSocket);
    void processRequest(int clientSocket, const HttpRequest& request);
    void sendResponse(int clientSocket, const HttpResponse& response);
    void handleError(int clientSocket, const HttpError& error);
    
    static constexpr size_t BUFFER_SIZE = 8192;
    static constexpr int SOCKET_TIMEOUT_SECONDS = 30;
};

#endif // SERVER_H