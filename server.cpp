#include "server.h"
#include <iostream>
#include <ws2tcpip.h>
#include <sstream>
#include <fstream>
#include <thread>
#include "router.h"

#pragma comment(lib, "ws2_32.lib")

// Constructor
Server::Server(int port) {
    this->port = port;

    router.use([](const std::string& method, const std::string& path) {
        std::cout << " " << method << " " << path << std::endl;
    });

    //  Middleware 2 — Fake Auth check
    router.use([](const std::string& method, const std::string& path) {
        if (path == "/admin") {
            std::cout << " Checking admin access...\n";
        }
    });

    //  ROUTES (Express-style)

    // GET /api
    router.get("/api", [](const std::string& body) {
        return "{\"message\": \"GET API \"}";
    });

    // POST /api
    router.post("/api", [](const std::string& body) {
        std::string name = "User";

        if (body.find("name") != std::string::npos) {
            size_t start = body.find(":") + 2;
            size_t end = body.find("\"", start);
            name = body.substr(start, end - start);
        }

        return "{\"message\": \"Hello " + name + " \"}";
    });

    // GET /about
    router.get("/about", [](const std::string& body) {
        return "<html><body><h1>About Page</h1></body></html>";
    });

    router.get("/admin", [](const std::string& body) {
        return "<html><body><h1>Admin Panel </h1></body></html>";
    });
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

    //  Extract BODY (for POST)
    std::string bodyData = "";
    size_t pos = request.find("\r\n\r\n");
    if (pos != std::string::npos) {
        bodyData = request.substr(pos + 4);
    }

    std::cout << " Body: " << bodyData << std::endl;

    // Ignore favicon
    if (path == "/favicon.ico") {
        closesocket(client_socket);
        return;
    }

    //  ROUTER handles logic
    std::string body = router.route(method, path, bodyData);

    std::string contentType = "text/html";

    if (path == "/api") {
        contentType = "application/json";
    }

    //  Serve index.html for "/"
    if (path == "/") {
        std::ifstream file("index.html");

        if (file) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            body = buffer.str();
            contentType = "text/html";
        } else {
            body = "<html><body><h1>Home Page</h1></body></html>";
        }
    }

    //  Handle 404
    if (body == "404 Not Found") {
        body = "<html><body><h1>404 Not Found</h1></body></html>";
        contentType = "text/html";
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