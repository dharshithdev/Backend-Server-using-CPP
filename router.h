#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <functional>
#include <map>
#include <vector>

class Router {
public:
    using Handler = std::function<std::string(const std::string&)>;
    using Middleware = std::function<void(const std::string& method, const std::string& path)>;

    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);

    void use(Middleware middleware);  

    std::string route(const std::string& method, const std::string& path, const std::string& body);

private:
    std::map<std::string, Handler> getRoutes;
    std::map<std::string, Handler> postRoutes;

    std::vector<Middleware> middlewares; 
};

#endif