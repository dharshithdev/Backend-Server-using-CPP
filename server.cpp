#include "server.h"
#include <iostream>
#include <ws2tcpip.h>
#include <sstream>
#include <fstream>
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

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == INVALID_SOCKET) {
        std::cout << "Socket failed\n";
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cout << "Bind failed\n";
        return;
    }

    if (listen(server_fd, 10) == SOCKET_ERROR) {
        std::cout << "Listen failed\n";
        return;
    }

    std::cout << " Server running on port " << port << std::endl;

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

// Handle client
void Server::handleClient(SOCKET client_socket) {
    char buffer[30000] = {0};

    recv(client_socket, buffer, 30000, 0);

    std::string request(buffer);
    std::cout << "\n Request:\n" << request << std::endl;

    //  Parse METHOD + PATH
    std::istringstream iss(request);
    std::string method, path;
    iss >> method >> path;

    std::cout << " Method: " << method << " | Path: " << path << std::endl;

    //  Extract BODY (important for POST)
    std::string bodyData = "";
    size_t pos = request.find("\r\n\r\n");
    if (pos != std::string::npos) {
        bodyData = request.substr(pos + 4);
    }

    std::cout << "📦 Body: " << bodyData << std::endl;

    // Ignore favicon
    if (path == "/favicon.ico") {
        closesocket(client_socket);
        return;
    }

    std::string body;
    std::string contentType = "text/html";

    //  ROUTING

    // GET API
    if (method == "GET" && path == "/api") {
        contentType = "application/json";
        body = "{\"message\": \"GET API working \"}";
    }

    // POST API
    else if (method == "POST" && path == "/api") {
        contentType = "application/json";

        std::string name = "User";

        // Simple JSON parsing
        if (bodyData.find("name") != std::string::npos) {
            size_t start = bodyData.find(":") + 2;
            size_t end = bodyData.find("\"", start);
            name = bodyData.substr(start, end - start);
        }

        body = "{\"message\": \"Hello " + name + " \"}";
    }

    // ABOUT
    else if (path == "/about") {
        body = "<html><body><h1>About Page</h1></body></html>";
    }

    // HOME (serve file)
    else {
        std::ifstream file("index.html");

        if (file) {
            std::stringstream fileBuffer;
            fileBuffer << file.rdbuf();
            body = fileBuffer.str();
        } else {
            body = "<html><body><h1>Home Page</h1></body></html>";
        }
    }

    //  FINAL RESPONSE
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: " + contentType + "\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n\r\n" +
        body;

    send(client_socket, response.c_str(), response.size(), 0);

    closesocket(client_socket);
}