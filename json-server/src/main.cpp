#include <crow_all.h>
#include <iostream>
#include <map>
#include <string>
#include <mutex>
#include <nlohmann/json.hpp>

using Map = std::map<std::string, nlohmann::json>;
Map myDictionary;
std::mutex dictionaryMutex;

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/add").methods("POST"_method)([&myDictionary](const crow::request& req){
        try {
            auto jsonRequest = nlohmann::json::parse(req.body);

            std::string key = jsonRequest["key"];
            nlohmann::json value = jsonRequest["value"];

            {
                std::lock_guard<std::mutex> lock(dictionaryMutex);
                myDictionary[key] = value;
            }
            
            return crow::response{200, crow::json::wvalue{{"status", "success"}}};
        } catch (const std::exception& e) {
            return crow::response(400);
        }
    });

    CROW_ROUTE(app, "/get").methods("POST"_method)([&myDictionary](const crow::request& req){
    try {
        auto jsonRequest = nlohmann::json::parse(req.body);

        std::string key = jsonRequest["key"].get<std::string>();
        int index = jsonRequest.value("index", -1);

        std::lock_guard<std::mutex> lock(dictionaryMutex);

        auto it = myDictionary.find(key);

        if (it != myDictionary.end()) {
            if (index >= 0 && index < it->second.size() && it->second.is_array()) {
                return crow::response(200, it->second[index].dump());
            } else {
                return crow::response{200, it->second.dump()};
            }
        } else {
            return crow::response(404, crow::json::wvalue{{"status", "error"}, {"message", "Key not found"}});
        }
    } catch (const std::exception& e) {
        return crow::response(400, crow::json::wvalue{{"status", "error"}, {"message", "Invalid JSON format"}});
    }
});



    app.port(18080).multithreaded().run();
}
