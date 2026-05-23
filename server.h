#include <winsock2.h>

class Server {
private:
    int port;
    SOCKET server_fd;

public:
    Server(int port);
    void start();
    void handleClient(int client_socket);
};