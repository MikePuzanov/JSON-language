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
    if (!command.empty() && command[0].is_string() && isURL(command[0])) {
        string fullUrl = command[0].get<string>();
        json newCommand(command.begin() + 1, command.end());
        cout << "Information: New command Get - " + newCommand.dump() << endl;

        Client client(fullUrl);
        json payload = newCommand;
        auto response = client.Post("/get", payload.dump(), "application/json");

        validateResponse(fullUrl, response);
        
        return json::parse(response->body);        
    } else {
        lock_guard<mutex> lock(galaxyMutex);
        return processGet(mciSendCommandA);
    }
}

void JsonLanguage::add(const json& command) {
    if (command.empty() || command.size() != 2)
    {
        throw InvalidJSONFormatException("The command argument must be an array of two elements.", command);
    }
    
    json path = command[0];
    if (path[0].is_string() && isURL(path[0])) {
        string fullUrl = path[0].get<string>();
        json newPath(path.begin() + 1, path.end());
        json newCommand = { newPath, command[1] };
        cout << "Information: New command Add - " + newCommand.dump() << endl;

        Client client(fullUrl);
        json payload = newCommand;
        auto response = client.Post("/add", payload.dump(), "application/json");
                    
        validateResponse(fullUrl, response);
    } else {
        lock_guard<mutex> lock(galaxyMutex);
        cout << "Invoke Add" << endl;
        processAdd(command[0], command[1]);
    }
}

void JsonLanguage::remove(const json& command) {
    cout << "BODY: - " + command.dump() << endl;
    if (!command.empty() && command[0].is_string() && isURL(command[0])) {
        string fullUrl = command[0].get<string>();
        json newCommand(command.begin() + 1, command.end());
        cout << "Information: New command Delete- " + newCommand.dump() << endl;

        Client client(fullUrl);
        json payload = newCommand;
        auto response = client.Delete("/delete", payload.dump(), "application/json");

        validateResponse(fullUrl, response);      
    } else {
        lock_guard<mutex> lock(galaxyMutex);
        return processDelete(command);
    }
}

void JsonLanguage::validateResponse(string fullUrl, Result &response) {
    if (!response) {
        throw FailedConnectionException("Error: Failed connection to server. Url: ", fullUrl);
    }

    string responseBody = response->body;

    switch (response->status) {
        case 200:
            return;
        case 404:
            throw NotFoundDataServerException("Error: Not found data on server. Url: " , responseBody, fullUrl);
        case 400:
            if (responseBody.find("Error: Index out of array lenght") != std::string::npos) {
                throw IndexServerException("Server: Index out of array lenght", responseBody, fullUrl);
            } else if (responseBody.find("Error: To select in an array you need a numeric index.") != std::string::npos) {
                throw IsNotArrayServerException("Server: Accessing by index not to an array", responseBody, fullUrl);
            } else {
                throw InvalidJSONFormatServerException("Server: Invalid JSON format.", responseBody, fullUrl);
            }
        case 422:
            throw InvalidCombinationServerException("Server: Not valid request body. Request body should be array with two elements.", responseBody, fullUrl);
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
                    throw IndexException("Error: Index out of array length.", result.dump(), result.size());
                }
            } else {
                throw IsNotArrayException("Error: To delete from an array you need a numeric index.", result.dump());
            }
        } else {
            throw NotFoundDataException("Error: Not found data.", step.dump());
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
            cerr << "Error: Invalid JSON format in combination" << endl;
            throw InvalidCombinationException("Error: Invalid JSON format in combination.", step.dump());
        }
    }

    *currentLevel = result;
}

// to delete data from galaxy
void JsonLanguage::processDelete(const json& query) {
    json* result = &galaxy;

    if (query.size() == 0) {
        cout << "FULL DELETE";
        *result = json();
        return;
    }

    for (const auto& step : query) {
        if (result->is_object()) {
            if (step.is_string() && result->find(step.get<string>()) != result->end()) {
                if (&step == &query.back()) {
                    result->erase(step.get<string>());
                    return;
                } else {
                    result = &((*result)[step.get<string>()]);
                }
            } else {
                throw NotFoundDataException("Error: Not found data.", step.dump());
            }
        } else if (result->is_array()) {
            if (step.is_number_integer()) {
                size_t index = step.get<size_t>();
                if (index >= 0 && index < result->size()) {
                    if (&step == &query.back()) {
                        result->erase(result->begin() + index);
                        return;
                    } else {
                        result = &((*result)[index]);
                    }
                } else {
                    throw IndexException("Error: Index out of array length.", result->dump(), result->size());
                }
            } else {
                throw IsNotArrayException("Error: To delete from an array you need a numeric index.", result->dump());
            }
        } else {
            throw NotFoundDataException("Error: Not found data.", step.dump());
        }
    }
}

bool JsonLanguage::isURL(const string &str) {
    // check URL
    regex urlRegex("(https?|ftp)://[\\w\\-_]+(\\.[\\w\\-_]+)+([a-zA-Z0-9\\-.,@?^=%&:/~+#]*[a-zA-Z0-9\\-@?^=%&/~+#])?");
    return regex_match(str, urlRegex);
}