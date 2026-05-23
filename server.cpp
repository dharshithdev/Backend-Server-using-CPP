#include "server.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <thread>

#pragma comment(lib, "ws2_32.lib")  

// Constructor
Server::Server(int port) {
    this->port = port;
}

// Start server
void Server::start() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);  

    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == INVALID_SOCKET) {
        std::cout << "Socket failed\n";
        return;
    }

    // 2. Set address + port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // 3. Bind
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cout << "Bind failed\n";
        return;
    }

    // 4. Listen
    if (listen(server_fd, 10) == SOCKET_ERROR) {
        std::cout << "Listen failed\n";
        return;
    }

    std::cout << " Server running on port " << port << std::endl;

    // 5. Accept loop
    while (true) {
        SOCKET client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);

        if (client_socket == INVALID_SOCKET) {
            std::cout << "Accept failed\n";
            continue;
        }

        std::thread(&Server::handleClient, this, client_socket).detach();
    }

    closesocket(server_fd);
    WSACleanup();
}

// Handle each client
void Server::handleClient(int client_socket) {
    char buffer[30000] = {0};

    recv(client_socket, buffer, 30000, 0);

    std::string request(buffer);

    std::cout << "\n Request:\n" << request << std::endl;

    std::string body;
    std::string response;

    // ✅ ROUTING
    if (request.find("GET /api") != std::string::npos) {

        std::cout << "Matched /api\n";

        body = "{\"message\": \"Hello from C++ API \"}";

        response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n\r\n" +
            body;
    }
    else if (request.find("GET /about") != std::string::npos) {

        std::cout << "Matched /about\n";

        body = "<html><body><h1>About Page</h1></body></html>";

        response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n\r\n" +
            body;
    }
    else {

        std::cout << "Matched default\n";

        body = "<html><body><h1> Home Page</h1></body></html>";

        response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n\r\n" +
            body;
    }

    send(client_socket, response.c_str(), response.size(), 0);

    closesocket(client_socket);
}