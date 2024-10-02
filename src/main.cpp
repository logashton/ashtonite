#include "server.h"
#include <iostream>

int main()
{
    Server server(8080, "127.0.0.1");
    
    server.start();
    server.acceptConnection();

    std::string message;

    while (true) {
        message = server.receiveMessage();
        
        if (message.size() <= 0) {
            continue;
        }

        std::cout << "Message from client: " << message << std::endl;
    }

    server.close();
    return 0;
}

