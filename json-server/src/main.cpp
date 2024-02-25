#include <crow.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <fstream>
#include <iostream>

using json = nlohmann::json;
nlohmann::json galaxy;
std::mutex galaxyMutex;

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
                    return json::object({{"status", "error"}, {"message", "Нет такого индекса"}});
                }
            } else {
                return json::object({{"status", "error"}, {"message", "Для выбора в массиве нужен числовой индекс"}});
            }
        } else {
            // Если условия не выполнились, возвращаем ошибку
            return json::object({{"status", "error"}, {"message", "Нет такого поля"}});
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
            return;
        }
    }

    *currentLevel = result;
}

int main() {
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
                    return crow::response{404, result.dump()};
                } else {
                    return crow::response{200, result.dump()};
                }
            } else {
                // Если запрос не является массивом или объектом, возвращаем ошибку
                return crow::response{400, "Неправильный формат JSON"};
            }
        } catch (const std::exception& e) {
            // Если произошла ошибка при парсинге JSON, возвращаем ошибку
            return crow::response{400, "Неправильный формат JSON"};
        }
    });

// Эндпоинт для создания/обновления объекта JSON по указателю
CROW_ROUTE(app, "/add").methods("POST"_method)([](const crow::request& req) {
    try {
        auto jsonRequest = json::parse(req.body);

        if (jsonRequest.is_array()) {
            std::lock_guard<std::mutex> lock(galaxyMutex);

            processAdd(jsonRequest[0], jsonRequest[1]);           

            return crow::response{200, "Success"};
        } else {
            return crow::response{400, "Неправильный формат JSON"};
        }
    } catch (const std::exception& e) {
        return crow::response{400, "Неправильный формат JSON"};
    }
});

    app.port(18080).multithreaded().run();
}