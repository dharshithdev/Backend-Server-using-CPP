#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>

class Server {
private:
    int port;
    SOCKET server_fd;

public:
    Server(int port);
    void start();
    void handleClient(SOCKET client_socket);
};

#endif