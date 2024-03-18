#include "JsonLanguage.h"
#include <nlohmann/json.hpp>
#include "httplib.h"
#include <regex>

using namespace std;
using namespace nlohmann;
using namespace httplib;
json galaxy;
mutex galaxyMutex;

json JsonLanguage::get(const json& command) {
    // Проверяем, есть ли URL в команде
    cout << "Команда -" + command.dump() << endl;
    if (!command.empty() && command[0].is_string() && isURL(command[0])) {
        string fullUrl = command[0].get<string>();
        cout << fullUrl << endl;
        json newCommand(command.begin() + 1, command.end());
        cout << "Новая команда - " + newCommand.dump() << endl;

        // Отправляем HTTP-POST-запрос
        Client client(fullUrl);
        json payload = newCommand;
        auto response = client.Post("/get", payload.dump(), "application/json");

        validateResponse(fullUrl, response);
        
        return json::parse(response->body);        
    } else {
        // Вызываем функцию для обработки запроса локально
        lock_guard<mutex> lock(galaxyMutex);
        return processGet(command, galaxy);
    }
}

void JsonLanguage::add(const json& command) {
    if (command.empty() || command.size() != 2)
    {
        throw InvalidJSONFormatException("При попытке записи JSON-массив должен состоять из двух элементов.", command);
    }
    
    json path = command[0];
    // Проверяем, есть ли URL в команде
    if (path[0].is_string() && isURL(path[0])) {
        string fullUrl = path[0].get<string>();
        json newPath(path.begin() + 1, path.end());
        json newCommand = { newPath, command[1] };
        cout << "Новая команда - " + newCommand.dump() << endl;

        // Отправляем HTTP-POST-запрос
        Client client(fullUrl);
        json payload = newCommand;
        auto response = client.Post("/add", payload.dump(), "application/json");
                    
        validateResponse(fullUrl, response);
    } else {
        // Вызываем функцию для обработки добавления локально
        lock_guard<mutex> lock(galaxyMutex);
        processAdd(command[0], command[1]);
    }
}

void JsonLanguage::validateResponse(string fullUrl, Result &response) {
    if (!response) {
        throw FailedConnectionException("Ошибка соединения с сервером.", fullUrl);
    }

    string responseBody = response->body;

    switch (response->status) {
        case 200:
            return;
        case 404:
            throw NotFoundDataServerException("Не удалось найти данные на сервере." , responseBody, fullUrl);
        case 400:
            if (responseBody.find("Выход за рамеры массива") != std::string::npos) {
                throw IndexServerException("Выход за рамеры массива", responseBody, fullUrl);
            } else if (responseBody.find("Для выбора в массиве нужен числовой индекс") != std::string::npos) {
                throw IsNotArrayServerException("Для выбора в массиве нужен числовой индекс", responseBody, fullUrl);
            } else {
                throw InvalidJSONFormatServerException("Невалидный JSON отправлен на сервер.", responseBody, fullUrl);
            }
        case 422:
            throw InvalidCombinationServerException("Невалидное тело запроса.", responseBody, fullUrl);
        default:
            throw ServerException("Unexpected server response.", responseBody);
    }
}

json JsonLanguage::processGet(const json& query, const json& current) {
    json result = current;

    if (query.empty()) {
        return galaxy;
    }

    for (const auto& step : query) {
        if (result.is_object() && step.is_string() && result.find(step.get<string>()) != result.end()) {
           result = result.at(step.get<string>());
        } else if (result.is_array()) {
            if (step.is_number_integer()) {
                if (step >= 0 && step < result.size()) {
                result = result[step.get<size_t>()];
                }
                else {
                    throw IndexException("Выход за рамеры массива.", result.dump(), result.size());
                }
            } else {
                throw IsNotArrayException("Для выбора в массиве нужен числовой индекс.", result.dump());
            }
        } else {
            // Если условия не выполнились, возвращаем ошибку
            throw NotFoundDataException("Нет такого поля.", step.dump());
        }
    }

    return result;
}

void JsonLanguage::processAdd(const json& command, const json& result) {
    json& current = galaxy;

    json* currentLevel = &current;

    for (const auto& step : command) {
        if (step.is_string()) {
            if (currentLevel->is_object() && currentLevel->find(step.get<string>()) != currentLevel->end()) {
                currentLevel = &(*currentLevel)[step.get<string>()];
            } else {
                (*currentLevel)[step.get<string>()] = json::object();
                currentLevel = &(*currentLevel)[step.get<string>()];
            }
        } else if (step.is_number() && currentLevel->is_array()) {
            size_t index = step;
            if (index < currentLevel->size()) {
                currentLevel = &(*currentLevel)[index];
            } else {
                currentLevel->push_back(nlohmann::json::object());
                currentLevel = &(*currentLevel)[index];
            }
        } else {
            // Некорректный путь, возвращаем ошибку
            cerr << "Error: Неверная команда" << endl;
            throw InvalidCombinationException("Неверный комбинация.", step.dump());
        }
    }

    *currentLevel = result;
}

bool JsonLanguage::isURL(const string &str) {
    // Регулярное выражение для проверки строки URL
    regex urlRegex("(https?|ftp)://[\\w\\-_]+(\\.[\\w\\-_]+)+([a-zA-Z0-9\\-.,@?^=%&:/~+#]*[a-zA-Z0-9\\-@?^=%&/~+#])?");
    return regex_match(str, urlRegex);
}