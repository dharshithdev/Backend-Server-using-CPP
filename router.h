#ifndef ROUTER_H
#define ROUTER_H

#include <string>
#include <functional>
#include <map>

class Router {
public:
    using Handler = std::function<std::string(const std::string& body)>;

    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);

    std::string route(const std::string& method, const std::string& path, const std::string& body);

private:
    std::map<std::string, Handler> getRoutes;
    std::map<std::string, Handler> postRoutes;
};

#endif