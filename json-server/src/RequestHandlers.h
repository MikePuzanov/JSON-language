#include "GalaxyServer.h"
#include <crow.h>

class RequestHandler {

public:
    RequestHandler(GalaxyServer& server) : galaxyServer(server) {}

    crow::response GetHandler(const crow::request& req);
    crow::response AddHandler(const crow::request& req);
    crow::response DeleteHandler(const crow::request& req);

private:
    GalaxyServer& galaxyServer;
};