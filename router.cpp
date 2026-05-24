#include "router.h"
#include <iostream>

// Register routes
void Router::get(const std::string& path, Handler handler) {
    getRoutes[path] = handler;
}

void Router::post(const std::string& path, Handler handler) {
    postRoutes[path] = handler;
}

// Register middleware
void Router::use(Middleware middleware) {
    middlewares.push_back(middleware);
}

// Route handler
std::string Router::route(const std::string& method, const std::string& path, const std::string& body) {

    for (auto& mw : middlewares) {
        if (!mw(method, path)) {
            return "403 Forbidden"; 
        }
    }

    // ROUTES
    if (method == "GET" && getRoutes.count(path)) {
        return getRoutes[path](body);
    }

    if (method == "POST" && postRoutes.count(path)) {
        return postRoutes[path](body);
    }

    return "404 Not Found";
}