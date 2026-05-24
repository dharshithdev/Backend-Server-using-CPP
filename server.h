#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include "router.h"   // ✅ ADD THIS

class Server {
private:
    int port;
    SOCKET server_fd;

    Router router; 

public:
    Server(int port);
    void start();
    void handleClient(SOCKET client_socket);
};

#endif