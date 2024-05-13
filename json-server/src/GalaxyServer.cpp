#include <crow.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <fstream>
#include <iostream>
#include "MyExceptions.h"
#include "GalaxyServer.h"
#include <asio.hpp>
#include <csignal>
#include <chrono>
#include <thread>

using namespace std;
using namespace nlohmann;
json galaxy;
mutex galaxyMutex;
mutex fileMutex;

// Const WINDOWS
const string galaxyFileNameWindows = "galaxy.json";
const string configFileNameWindows = "serverConfig.json";
// Const Ubuntu dev/testing
const string galaxyFileNameUbuntu = "galaxy.json";
const string configFileNameUbuntu = "serverConfig.json";
// Const Ubuntu for .deb pack 
//const string galaxyFileNameUbuntu = "/usr/bin/jsonServer/galaxy.json";
//const string configFileNameUbuntu = "/usr/bin/jsonServer/serverConfig.json";

json GalaxyServer::getGalaxy() {
    return galaxy;
}

json GalaxyServer::loadConfig() {
    try {
#ifdef _WIN32
        ifstream file(configFileNameWindows);
#else
        ifstream file(configFileNameUbuntu);
#endif
        if (!file.is_open()) {
            throw runtime_error("Failed to open config file: " + configFileNameWindows);
        }
        json config;
        file >> config;
        return config;
    }
    catch (const exception& e) {
        cerr << e.what(); 
    }
}

// load local galaxy from file
void GalaxyServer::loadDataFromGalaxyJson() {
    try {
#ifdef _WIN32
        ifstream file(galaxyFileNameWindows);
#else
        ifstream file(galaxyFileNameUbuntu);
#endif
        if (!file.is_open()) {
            throw runtime_error("Failed to open config file: " + galaxyFileNameWindows);
        }
        
        galaxy = json::parse(file);
        file.close();
        return;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what(); 
    }
}

// save data from galaxy to file
void GalaxyServer::saveGalaxyToFile() {
    try {
#ifdef _WIN32
        ofstream file(galaxyFileNameWindows);
#else
        ofstream file(galaxyFileNameUbuntu);
#endif
    
        if (!file.is_open()) {
            throw runtime_error("Failed to open config file: " + galaxyFileNameWindows);
        }
    
        lock_guard<mutex> lock(fileMutex);
        file << galaxy.dump(4);         
        file.close();
        cout << endl << "Information: Saved data to " << galaxyFileNameWindows << endl;
    } catch(const exception& e) {
        cerr << "Error: " << e.what(); 
    }
}

// to get data from galaxy
json GalaxyServer::processGet(const json& query) {
    lock_guard<mutex> lock(galaxyMutex);
    json result = galaxy;
    for (const auto& step : query) {
        if (result.is_object() && step.is_string() && result.find(step.get<string>()) != result.end()) {
           result = result.at(step.get<string>());
        } else if (result.is_array()) {
            if (step.is_number_integer()) {
                if (step >= 0 && step < result.size()) {
                    result = result[step.get<size_t>()];
                }
                else {
                    throw IndexException("Error: Index out of array lenght. Key: " + result.dump());
                }
            } else {
                throw IsNotArrayException("Error: To select in an array you need a numeric index. Key: " + result.dump());
            }
        } else {
            throw NotFoundDataException("Error: Not found data. Key: " + step.dump());
        }
    }

    return result;
}

// to add data in galaxy
void GalaxyServer::processAdd(const json& command, const json& result) {
    lock_guard<mutex> lock(galaxyMutex);
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
            cerr << "Error: Invalid JSON format" << endl;
            throw InvalidJSONFormatException("Error: Invalid JSON format. Key: " + step.dump());
        }
    }

    *currentLevel = result;
}

// to delete data from galaxy
void GalaxyServer::processDelete(const json& query) {
    lock_guard<mutex> lock(galaxyMutex);
    json* result = &galaxy;

    if (query.size() == 0) {
        *result = json();
        return;
    }

    for (const auto& step : query) {
        if (result->is_object()) {
            if (step.is_string() && result->find(step.get<string>()) != result->end()) {
                if (&step == &query.back()) { // Если это последний элемент запроса
                    result->erase(step.get<string>()); // Удаляем элемент из объекта
                    return;
                } else {
                    result = &((*result)[step.get<string>()]); // Переходим к следующему уровню
                }
            } else {
                throw NotFoundDataException("Error: Not found data. Key: " + step.dump());
            }
        } else if (result->is_array()) {
            if (step.is_number_integer()) {
                size_t index = step.get<size_t>();
                if (index >= 0 && index < result->size()) {
                    if (&step == &query.back()) { // Если это последний элемент запроса
                        result->erase(result->begin() + index); // Удаляем элемент из массива
                        return;
                    } else {
                        result = &((*result)[index]); // Переходим к следующему уровню
                    }
                } else {
                    throw IndexException("Error: Index out of array length. Key: " + result->dump());
                }
            } else {
                throw IsNotArrayException("Error: To delete from an array you need a numeric index. Key: " + result->dump());
            }
        } else {
            throw NotFoundDataException("Error: Not found data. Key: " + step.dump());
        }
    }
}

void GalaxyServer::setSignal() {
#ifdef _WIN32
    // setup signal for Windows
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "Error: with setup signal habdler" << endl;
    }
#else
    // setup signal for  Linux
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGSTOP, signalHandler);
#endif
}

#ifdef _WIN32
#include <Windows.h>

// signal for Windows
BOOL WINAPI GalaxyServer::ConsoleHandler(DWORD signal) {
    switch (signal) {
        case CTRL_C_EVENT:
            shutDownWindows();
            exit(signal);
        case CTRL_BREAK_EVENT:
            shutDownWindows();
            exit(signal);
        case CTRL_CLOSE_EVENT:
            shutDownWindows();
            exit(signal);
        case CTRL_SHUTDOWN_EVENT:
            shutDownWindows();
            exit(signal);
        case CTRL_LOGOFF_EVENT:
            shutDownWindows();
            exit(signal);
        default:
            return FALSE;
    }
}
#else
#include <unistd.h>

// signal for Linux
void GalaxyServer::signalHandler(int signal) {
    cout << "Information: Shutting down the server and saving data in galaxy.json. Signal: " << signal << endl;
    saveGalaxyToFile();
    exit(signal);
}
#endif

void GalaxyServer::shutDownWindows() {
    cout << "Information: Shutting down the server and saving data in galaxy.json. Signal: " << signal << endl;
    saveGalaxyToFile();
}