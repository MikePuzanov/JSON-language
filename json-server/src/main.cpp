#include <crow.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <iostream>
#include <asio.hpp>
#include <thread>
#include "GalaxyServer.h"
#include "RequestHandlers.h"

using namespace std;
using namespace nlohmann;



int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    GalaxyServer galaxyServer;
    RequestHandler requestHandler = RequestHandler(galaxyServer);
    std::string host = "";
    int port = 0;

    if (argc != 3) {
        json config = galaxyServer.loadConfig();
        host = config["ip"];
        port = config["port"];
    } else {
        host = argv[1];
        port = stoi(argv[2]);
    }

    galaxyServer.loadDataFromGalaxyJson();
    galaxyServer.setSignal();

    crow::SimpleApp app;

    // POST /get
    CROW_ROUTE(app, "/get").methods("POST"_method)([&requestHandler](const crow::request& req) {
        return requestHandler.GetHandler(req);
    });

    // POST /add
    CROW_ROUTE(app, "/add").methods("POST"_method)([&requestHandler](const crow::request& req) {
        return requestHandler.AddHandler(req);
    });

    // Delete /delete
    CROW_ROUTE(app, "/delete").methods("DELETE"_method)([&requestHandler](const crow::request& req) {
        return requestHandler.DeleteHandler(req);
    });

    galaxyServer.addSaveCronJob();
   
    app.bindaddr(host).port(port).multithreaded().run();

    galaxyServer.joinSaveCronThread();
   
    return 0;
}