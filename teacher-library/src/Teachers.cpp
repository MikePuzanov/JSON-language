#include "Teachers.h"
#include <json.hpp>
#include "httplib.h"

using namespace std;
using namespace nlohmann;
using namespace httplib;

Teachers::Teachers(const string& initialFileName) : fileName(initialFileName) {
    init();
}

void Teachers::init() {
    ifstream file(this->fileName);
    json data;

    if (file.is_open()) {    
        cout << endl << "Open File" << endl;
        data = json::parse(file);
        file.close();
    } else {
        cout << endl << "Create File" << endl;
        ofstream file(this->fileName);
        data = json({});
    }

    this->data = data;
}

// Определение метода для получения имени
string Teachers::getName() const {
    return fileName;
}

json Teachers::get(json& query) {
    json value = processGet(query);
    if (isURL(value)) {
        query.insert(query.begin(), value);
        json result = getFunc(query);
        return result;
    }

    return value;
}

void Teachers::addToServers(json& query, const json& value, const json& urls) {
    if (urls.is_null() || urls.empty()) {
        processAdd(query, value);
        saveGalaxyToFile();
        return;
    }
    processAdd(query, urls);
    saveGalaxyToFile();
    for (const auto& url : urls) {
        query.insert(query.begin(), url);
        json newQuery = json::array();
        newQuery.push_back(query);
        newQuery.push_back(value);
        addFunc(newQuery);
    }
}

void Teachers::remove(const json& query) {
    json value = processGet(query);
    if (isURL(value)) {
        json newQuery = query;
        newQuery.insert(newQuery.begin(), value);
        removeFunc(newQuery);
    }
    processDelete(query);
    saveGalaxyToFile();
}

json Teachers::getFunc(const json& command) {
    if (!command.empty() && command[0].is_string() && isURL(command[0])) {
        string fullUrl = command[0].get<string>();
        json newCommand(command.begin() + 1, command.end());
        //cout << "Information: New command Get - " + newCommand.dump() << endl;

        Client client(fullUrl);
        json payload = newCommand;
        auto response = client.Post("/get", payload.dump(), "application/json");

        validateResponse(fullUrl, response);
        
        return json::parse(response->body);        
    } else {
        cout << "First element is not url. Sequence: " << command.dump() << endl;
    }
}

void Teachers::addFunc(const json& command) {
    if (command.empty() || command.size() != 2)
    {
        throw InvalidJSONFormatException("The command argument must be an array of two elements.", command);
    }
    
    json path = command[0];
    if (path[0].is_string() && isURL(path[0])) {
        string fullUrl = path[0].get<string>();
        json newPath(path.begin() + 1, path.end());
        json newCommand = { newPath, command[1] };
        //cout << "Information: New command Add - " + newCommand.dump() << endl;

        Client client(fullUrl);
        json payload = newCommand;
        auto response = client.Post("/add", payload.dump(), "application/json");
                    
        validateResponse(fullUrl, response);
    } else {
        cout << "First element is not url. Sequence: " << command.dump() << endl;
    }
}

void Teachers::removeFunc(const json& command) {
    if (!command.empty() && command[0].is_string() && isURL(command[0])) {
        string fullUrl = command[0].get<string>();
        json newCommand(command.begin() + 1, command.end());
        //cout << "Information: New command Delete- " + newCommand.dump() << endl;

        Client client(fullUrl);
        json payload = newCommand;
        auto response = client.Delete("/delete", payload.dump(), "application/json");

        validateResponse(fullUrl, response);      
    } else {
        cout << "First element is not url. Sequence: " << command.dump() << endl;
    }
}

void Teachers::validateResponse(string fullUrl, Result &response) {
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

json Teachers::processGet(const json& query) {
    json result = this->data;

    if (query.empty()) {
        return data;
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

void Teachers::processAdd(const json& command, const json& result) {
    json& current = this->data;

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

    if (currentLevel->empty()) {
        *currentLevel = result;
        return;
    }

    json new_level = json::array();
    new_level.push_back(*currentLevel);
    if (result.is_array())
    {
        new_level.insert(new_level.end(), result.begin(), result.end());
    } else {
        new_level.push_back(result);
    }
    
    *currentLevel = new_level;
}

// to delete data from galaxy
void Teachers::processDelete(const json& query) {
    json* result = &data;

    if (query.size() == 0) {
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

bool Teachers::isURL(const string &str) {
    // check URL
    regex urlRegex("(https?|ftp)://[\\w\\-_]+(\\.[\\w\\-_]+)+([a-zA-Z0-9\\-.,@?^=%&:/~+#]*[a-zA-Z0-9\\-@?^=%&/~+#])?");
    return regex_match(str, urlRegex);
}

void Teachers::saveGalaxyToFile() {
    try {
        ofstream file(this->fileName);
    
        if (!file.is_open()) {
            throw runtime_error("Failed to open config file: " + this->fileName);
        }
    
        file << this->data.dump(4);         
        file.close();
        cout << endl << "Information: Saved data to " << this->fileName << endl;
    } catch(const exception& e) {
        cerr << "Error: " << e.what(); 
    }
}