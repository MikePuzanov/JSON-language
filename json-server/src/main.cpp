#include <crow.h>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>
#include <memory>
#include <fstream>
#include "json.hpp"
#include <mutex>

using json = nlohmann::json;
using Map = std::map<std::string, double>; 
Map myDictionary;
std::mutex dictionaryMutex;


int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/add_json").methods("POST"_method)([&myDictionary](const crow::request& req){
        try {
            auto jsonRequest = json::parse(req.body);

            std::string key = jsonRequest["key"];

            {
                std::lock_guard<std::mutex> lock(dictionaryMutex);
                myDictionary[key] = value;
            }
            
            return crow::response{json{{"status", "success"}}.dump()};
        } catch (const std::exception& e) {
            return crow::response(400);
        }
    });

    CROW_ROUTE(app, "/get").methods("POST"_method)([&myDictionary](const crow::request& req){
        try {
            auto jsonRequest = json::parse(req.body);

            std::string key = jsonRequest["key"];

            {
                std::lock_guard<std::mutex> lock(dictionaryMutex);

                auto dictEnd = myDictionary.end();

                auto it = myDictionary.find(key);
            }

            if (it != dictEnd) {
                    return crow::response{json{{"status", "success"}, {"value", it->second}}.dump()};
            } else {
                    return crow::response(404, json{{"status", "error"}, {"message", "Key not found"}});
            }
        } catch (const std::exception& e) {
                return crow::response(400, json{{"status", "error"}, {"message", "Invalid JSON format"}});
        }
    });

    app.port(18080).multithreaded().run();
}
