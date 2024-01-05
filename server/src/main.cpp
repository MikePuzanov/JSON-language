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
using json = nlohmann::json;

using Map = std::map<std::string, double>; 

Map myDictionary;


int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/add_json").methods("POST"_method)([&myDictionary](const crow::request& req){
        try {
            // Парсинг JSON из тела запроса
            auto jsonRequest = json::parse(req.body);

            // Извлечение ключа из JSON
            std::string key = jsonRequest["key"];

            // Добавление ключа и значения в словарь
            myDictionary[key] = jsonRequest["value"]; // Пример, установите нужное значение
            
            // Возвращение ответа в виде JSON
            return crow::response{json{{"status", "success"}}.dump()};
        } catch (const std::exception& e) {
            return crow::response(400); // Ошибка парсинга JSON
        }
    });

    CROW_ROUTE(app, "/get").methods("POST"_method)([&myDictionary](const crow::request& req){
        try {
            // Парсинг JSON из тела запроса
            auto jsonRequest = json::parse(req.body);

            // Извлечение ключа из JSON
            std::string key = jsonRequest["key"];

            // Проверка наличия ключа в словаре
            auto it = myDictionary.find(key);
            if (it != myDictionary.end()) {
                // Возвращение ответа в виде JSON
                return crow::response{json{{"status", "success"}, {"value", it->second}}.dump()};
            } else {
                // Ключ не найден
                return crow::response{json{{"status", "error"}, {"message", "Key not found"}}.dump()};
            }
        } catch (const std::exception& e) {
            return crow::response(400); // Ошибка парсинга JSON
        }
    });

    app.port(18080).multithreaded().run();
}
