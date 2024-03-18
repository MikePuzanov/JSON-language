#include <crow.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <fstream>
#include <iostream>
#include "myExceptions.h"

using namespace std;
using namespace nlohmann;
json galaxy;
mutex galaxyMutex;

json loadConfig(const string& configFile) {
    try {
        ifstream file(configFile);
    if (!file.is_open()) {
        throw runtime_error("Failed to open config file: " + configFile);
    }
    json config;
    file >> config;
    return config;
    }
    catch (const exception& e) {
        cerr << e.what(); 
    }
}

void saveGalaxy() {
    ofstream file("galaxy.json");
    file << galaxy.dump(4);
}

json processGet(const json& query) {
    json result = galaxy;
    cout << "Galaxy - " + galaxy.dump() << endl;
    for (const auto& step : query) {
        if (result.is_object() && step.is_string() && result.find(step.get<string>()) != result.end()) {
           result = result.at(step.get<string>());
        } else if (result.is_array()) {
            if (step.is_number_integer()) {
                if (step >= 0 && step < result.size()) {
                result = result[step.get<size_t>()];
                }
                else {
                    throw IndexException("����� �� ������ �������. M�����: " + result.dump());
                }
            } else {
                throw IsNotArrayException("��� ������ � ������� ����� �������� ������. M�����: " + result.dump());
            }
        } else {
            // ���� ������� �� �����������, ���������� ������
            throw NotFoundDataException("��� ������ ����. ����: " + step.dump());
        }
    }

    return result;
}

void processAdd(const json& command, const json& result) {
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
                currentLevel->push_back(json::object());
                currentLevel = &(*currentLevel)[index];
            }
        } else {
            // ������������ ����, ���������� ������
            cerr << "Error: �������� �������" << endl;
            throw InvalidJSONFormatException("�������� ������ JSON. Error: �������� ������� " + step.dump());
        }
    }

    *currentLevel = result;
}

int main() {
    setlocale(LC_ALL, "Russian");
    json config = loadConfig("serverConfig.json");
    string ip = config["ip"];
    int port = config["port"];

    // �������� ��������� ��������� �� �����
    // ifstream file("galaxy.json");
    // if (file.is_open()) {
    //     file >> galaxy;
    //     file.close();
    // }

    crow::SimpleApp app;

    // ���������� GET ������� �� ���� /get
    CROW_ROUTE(app, "/get").methods("POST"_method)([](const crow::request& req) {
        try {
            cout << endl << endl << "����� ������� Get" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "���� ������� " + jsonRequest.dump() << endl;


            if (jsonRequest.empty()) {
                return crow::response{200, galaxy.dump()};
            }

            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                lock_guard<mutex> lock(galaxyMutex);

                cout << "������� � �������" << endl;
                json result = processGet(jsonRequest);
                cout << "����� �������" << endl;

                if (result.is_object() && result.find("status") != result.end() && result["status"] == "error") {    
                    return crow::response{404, result.dump()};
                } else {
                    return crow::response{200, result.dump()};
                }
            } else {
                // ���� ������ �� �������� �������� ��� ��������, ���������� ������
                return crow::response{400, "������������ ������ JSON"};
            }
        } catch (const IndexException& e) {
            cout << "������� ������ IndexException." << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const IsNotArrayException& e) {
            cout << "������� ������ IsNotArrayException." << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const NotFoundDataException& e) {
            cout << "������� ������ NotFoundDataException." << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "������� ������." << endl;
            cerr <<  e.what();
            // ���� ��������� ������ ��� �������� JSON, ���������� ������
            return crow::response{400, e.what()};//"������������ ������ JSON"};
        }
    });

// �������� ��� ��������/���������� ������� JSON �� ���������
CROW_ROUTE(app, "/add").methods("POST"_method)([](const crow::request& req) {
    try {
        auto jsonRequest = json::parse(req.body);
        cout << "���� ������� " + jsonRequest.dump() << endl;
        
        if (jsonRequest.is_array()) {
            if (jsonRequest.size() != 2) {
                throw InvalidJSONFormatException("���� ������� ������ ��������� ������ �� 2 ���������. ���� = " + jsonRequest.dump());
            }
            lock_guard<mutex> lock(galaxyMutex);
            cout << "������� � ������� ������" << endl;
            processAdd(jsonRequest[0], jsonRequest[1]);           
            cout << "����� ������� ������" << endl;
            return crow::response{200, "Success"};
        } else {
            return crow::response{400, "������������ ������ JSON"};
        }
    } catch (const exception& e) {
        cout << "������� ������" << endl;
        return crow::response{400, "������������ ������ JSON"};
    } catch (const InvalidJSONFormatException& e) {
        cout << "������� ������ InvalidJSONFormatException." << endl;
        cerr <<  e.what();
        return crow::response{404, e.what()};
    }
});

   app.bindaddr(ip).port(port).multithreaded().run();
}