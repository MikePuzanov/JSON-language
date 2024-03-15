#include "json-language.h"
#include "MyExceptions.h"
#include <nlohmann/json.hpp>
#include "httplib.h"
#include <regex>

using namespace std;
nlohmann::json galaxy;
mutex galaxyMutex;

nlohmann::json MyLibrary::get(const nlohmann::json& command) {
    // ���������, ���� �� URL � �������
    if (!command.empty() && command[0].is_string() && isURL(command[0])) {
        try {
            string fullUrl = command[0].get<string>();
            cout << fullUrl << endl;
            nlohmann::json newCommand(command.begin() + 1, command.end());

            // ���������� HTTP-POST-������
            httplib::Client client(fullUrl.c_str());
            nlohmann::json payload = newCommand;
            payload.erase(payload.begin());
            auto response = client.Post("/get", payload.dump(), "application/json");

            validateResponse(fullUrl, response);
            
            return nlohmann::json::parse(response->body);
        } catch (const exception& e) {
            cerr << "Error: �������� �������" << endl;
        }
        
    } else {
        // �������� ������� ��� ��������� ������� ��������
        try
        {
            lock_guard<mutex> lock(galaxyMutex);
            return processGet(command, galaxy);
        }
        catch(const exception& e)
        {
            cerr << e.what() << '\n';
        }
    }
}

void MyLibrary::add(const nlohmann::json& command) {
    if (command.empty() || command.size() != 2)
    {
        throw InvalidJSONFormatException("��� ������� ������ JSON-������ ������ �������� �� ���� ���������. command = " + command);
    }
    
     //cout << command.dump() ;
    nlohmann::json path = command[0];
    // ���������, ���� �� URL � �������
    if (path[0].is_string() && isURL(path[0])) {
        try
        {
            string fullUrl = path[0].dump();
            cout << fullUrl << endl;
            nlohmann::json newPath(path.begin() + 1, path.end());
            cout << newPath.dump() << endl;
            nlohmann::json newCommand = { newPath, command[1] };
            cout << newCommand.dump() << endl;

            // ���������� HTTP-POST-������
            httplib::Client client(fullUrl.c_str());
            nlohmann::json payload = newCommand;
            payload.erase(payload.begin());
            auto response = client.Post("/add", payload.dump(), "application/json");
                        
            validateResponse(fullUrl, response);
        }
        catch(const exception& e)
        {
            cerr << e.what() << '\n';
        }
    } else {
        // �������� ������� ��� ��������� ���������� ��������
        try
        {
            lock_guard<mutex> lock(galaxyMutex);
            processAdd(command[0], command[1]);
        }
        catch(const exception& e)
        {
            cerr << e.what() << '\n';
        }
    }
}

void MyLibrary::validateResponse(string fullUrl, httplib::Result &response) {
    if (!response) {
        throw FailedConnectionException("������ ���������� � " + fullUrl);
    }

    switch (response->status) {
        case 200:
            return;
        case 404:
            throw NotFoundDataException("�� ������� ����� ������ �� ������� " + fullUrl + ". ���������: " + response->body);
        case 400:
            throw InvalidJSONFormatException(response->body);
        case 422:
            throw InvalidCombinationException("���������� ���� �������. ���������: " + response->body);
        default:
            throw ServerException("Unexpected server response.  ���������: " + response->body);
    }
}

nlohmann::json MyLibrary::processGet(const nlohmann::json& query, const nlohmann::json& current) {
    nlohmann::json result = current;

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
                    throw IndexException("����� �� ������ �������. M�����: " + result);
                    //return json::object({{"status", "error"}, {"message", indexException}});
                }
            } else {
                throw IsNotArrayException("��� ������ � ������� ����� �������� ������. M�����: " + result);
                //return json::object({{"status", "error"}, {"message", arrayException}});
            }
        } else {
            // ���� ������� �� �����������, ���������� ������
            throw NotFoundDataException("��� ������ ����. ����: " + step);
            //return json::object({{"status", "error"}, {"message", "��� ������ ����. ����: " + step}});
        }
    }

    return result;
}

void MyLibrary::processAdd(const nlohmann::json& command, const nlohmann::json& result) {
    nlohmann::json& current = galaxy;

    nlohmann::json* currentLevel = &current;

    for (const auto& step : command) {
        if (step.is_string()) {
            if (currentLevel->is_object() && currentLevel->find(step.get<string>()) != currentLevel->end()) {
                currentLevel = &(*currentLevel)[step.get<string>()];
            } else {
                (*currentLevel)[step.get<string>()] = nlohmann::json::object();
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
            // ������������ ����, ���������� ������
            cerr << "Error: �������� �������" << endl;
            throw InvalidJSONFormatException("�������� ������ JSON. Error: �������� ������� " + step);
        }
    }

    *currentLevel = result;
}

bool MyLibrary::isURL(const string &str) {
    // ���������� ��������� ��� �������� ������ URL
    regex urlRegex("(https?|ftp)://[\\w\\-_]+(\\.[\\w\\-_]+)+([a-zA-Z0-9\\-.,@?^=%&:/~+#]*[a-zA-Z0-9\\-@?^=%&/~+#])?");
    return regex_match(str, urlRegex);
}