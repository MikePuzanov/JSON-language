#include <crow.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <fstream>
#include <iostream>
#include "MyExceptions.h"

using json = nlohmann::json;
json galaxy;
std::mutex galaxyMutex;
const std::string indexException = "Выход за рамеры массива";
const std::string arrayException = "Для выбора в массиве нужен числовой индекс";

json loadConfig(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + configFile);
    }
    json config;
    file >> config;
    return config;
}

void saveGalaxy() {
    std::ofstream file("galaxy.json");
    file << galaxy.dump(4);
}

json processGet(const json& query, const json& current) {
    json result = current;

    for (const auto& step : query) {
        if (result.is_object() && step.is_string() && result.find(step.get<std::string>()) != result.end()) {
           result = result.at(step.get<std::string>());
        } else if (result.is_array()) {
            if (step.is_number_integer()) {
                if (step >= 0 && step < result.size()) {
                result = result[step.get<size_t>()];
                }
                else {
                    throw IndexException("Выход за рамеры массива. Mассив: " + result.dump());
                    //return json::object({{"status", "error"}, {"message", indexException}});
                }
            } else {
                throw IsNotArrayException("Для выбора в массиве нужен числовой индекс. Mассив: " + result.dump());
                //return json::object({{"status", "error"}, {"message", arrayException}});
            }
        } else {
            // Если условия не выполнились, возвращаем ошибку
            throw NotFoundDataException("Нет такого поля. Поле: " + step.dump());
            //return json::object({{"status", "error"}, {"message", "Нет такого поля. Поле: " + step}});
        }
    }

    return result;
}

void processAdd(const json& command, const json& result) {
    json& current = galaxy;
    json* currentLevel = &current;

    for (const auto& step : command) {
        if (step.is_string()) {
            if (currentLevel->is_object() && currentLevel->find(step.get<std::string>()) != currentLevel->end()) {
                currentLevel = &(*currentLevel)[step.get<std::string>()];
            } else {
                (*currentLevel)[step.get<std::string>()] = json::object();
                currentLevel = &(*currentLevel)[step.get<std::string>()];
            }
        } else if (step.is_number() && currentLevel->is_array()) {
            size_t index = step;
            if (index < currentLevel->size()) {
                currentLevel = &(*currentLevel)[index];
            } else {
                currentLevel->push_back(json::object());
                currentLevel = &(*currentLevel)[index];
            }
        } else {
            // Некорректный путь, возвращаем ошибку
            std::cerr << "Error: Неверная команда" << std::endl;
            throw InvalidJSONFormatException("Неверный формат JSON. Error: Неверная команда " + step.dump());
        }
    }

    *currentLevel = result;
}


int main() {
    json config = loadConfig("serverConfig.json");
    std::string ip = config["ip"];
    int port = config["port"];

    // Пытаемся загрузить галактику из файла
    std::ifstream file("galaxy.json");
    if (file.is_open()) {
        file >> galaxy;
        file.close();
    }

    crow::SimpleApp app;

    // Обработчик GET запроса по пути /get
    CROW_ROUTE(app, "/get").methods("POST"_method)([](const crow::request& req) {
        try {
            auto jsonRequest = json::parse(req.body);

            if (jsonRequest.empty()) {
                return crow::response{200, galaxy.dump()};
            }

            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                std::lock_guard<std::mutex> lock(galaxyMutex);

                json result = processGet(jsonRequest, galaxy);

                if (result.is_object() && result.find("status") != result.end() && result["status"] == "error") {
                    if (result.find("message") != result.end()) {
                        if (result["message"] == indexException || result["message"] == arrayException) {
                            return crow::response{422, result.dump()};
                        }
                    }
                    return crow::response{404, result.dump()};
                } else {
                    return crow::response{200, result.dump()};
                }
            } else {
                // Если запрос не является массивом или объектом, возвращаем ошибку
                return crow::response{400, "Неправильный формат JSON"};
            }
        } catch (const IndexException& e) {
            return crow::response{400, e.what()};
        } catch (const IsNotArrayException& e) {
            return crow::response{400, e.what()};
        } catch (const NotFoundDataException& e) {
            return crow::response{404, e.what()};
        } catch (const std::exception& e) {
            // Если произошла ошибка при парсинге JSON, возвращаем ошибку
                return crow::response{400, e.what()};//"Неправильный формат JSON"};
        }
    });

// Эндпоинт для создания/обновления объекта JSON по указателю
CROW_ROUTE(app, "/add").methods("POST"_method)([](const crow::request& req) {
    try {
        auto jsonRequest = json::parse(req.body);

        if (jsonRequest.is_array()) {
            if (jsonRequest.size() != 2) {
                throw InvalidJSONFormatException("Тело запроса должно содержать массив из 2 элементов. Тело = " + jsonRequest);
            }
            std::lock_guard<std::mutex> lock(galaxyMutex);

            processAdd(jsonRequest[0], jsonRequest[1]);           

            return crow::response{200, "Success"};
        } else {
            return crow::response{400, "Неправильный формат JSON"};
        }
    } catch (const std::exception& e) {
        return crow::response{400, "Неправильный формат JSON"};
    } catch (const InvalidJSONFormatException& e) {
        return crow::response{404, e.what()};
    }
});

   app.bindaddr(ip).port(port).multithreaded().run();
}