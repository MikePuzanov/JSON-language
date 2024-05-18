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
#include <atomic>

using namespace std;
using namespace nlohmann;

json galaxy;
mutex galaxyMutex;
mutex fileMutex;
std::atomic<bool> GalaxyServer::running(false);
mutex GalaxyServer::jobMutex;
condition_variable GalaxyServer::cvJob;
thread GalaxyServer::cronThread;
int GalaxyServer::saveCronJobTime = 0;

// Const WINDOW
const string galaxyFileNameWindows = "galaxy.json";
const string configFileNameWindows = "serverConfig.json";
// Const Ubuntu dev/testing
const string galaxyFileNameUbuntu = "galaxy.json";
const string configFileNameUbuntu = "serverConfig.json";
// Const Ubuntu for .deb pack 
//const string galaxyFileNameUbuntu = "/usr/bin/jsonServer/galaxy.json";
//const string configFileNameUbuntu = "/usr/bin/jsonServer/serverConfig.json";

GalaxyServer* GalaxyServer::instance = nullptr;

GalaxyServer::GalaxyServer() {
    running = false;
}

GalaxyServer::~GalaxyServer() {
    joinSaveCronThread();
}

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
        saveCronJobTime = config["saveCronTimeJob"];
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
                if (&step == &query.back()) { // ���� ��� ��������� ������� �������
                    result->erase(step.get<string>()); // ������� ������� �� �������
                    return;
                } else {
                    result = &((*result)[step.get<string>()]); // ��������� � ���������� ������
                }
            } else {
                throw NotFoundDataException("Error: Not found data. Key: " + step.dump());
            }
        } else if (result->is_array()) {
            if (step.is_number_integer()) {
                size_t index = step.get<size_t>();
                if (index >= 0 && index < result->size()) {
                    if (&step == &query.back()) { // ���� ��� ��������� ������� �������
                        result->erase(result->begin() + index); // ������� ������� �� �������
                        return;
                    } else {
                        result = &((*result)[index]); // ��������� � ���������� ������
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
    instance = this;
    #ifdef _WIN32
        if (!SetConsoleCtrlHandler(consoleHandler, TRUE)) {
            std::cerr << "Error: with setup signal handler" << std::endl;
        }
    #else
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
    #endif
}

void GalaxyServer::addSaveCronJob() {
    if (saveCronJobTime != 0) {
        running = true;
        cronThread = std::thread([this]() {
            std::unique_lock<std::mutex> lock(jobMutex);
            while (running) {
                if (cvJob.wait_for(lock, std::chrono::minutes(saveCronJobTime), [this] { return !running; })) {
                    break;
                }
                cout << "Information: Saving galaxy in galaxy.json from Job" << endl;
                saveGalaxyToFile();
            }
        });
    }
}

void GalaxyServer::joinSaveCronThread() {
    if (saveCronJobTime != 0) {
        {
            std::lock_guard<std::mutex> lock(jobMutex);
            running = false;
        }
        cvJob.notify_all();
        if (cronThread.joinable()) {
            cronThread.join();
        }
    }
}

#ifdef _WIN32
#include <Windows.h>

BOOL WINAPI GalaxyServer::consoleHandler(DWORD signal) {
    cout << "ASASDFAF";
    switch (signal) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_LOGOFF_EVENT:
            instance->shutDownWindows();
            return TRUE;
        default:
            return FALSE;
    }
}

void GalaxyServer::shutDownWindows() {
    std::cout << "Information: Shutting down the server and saving data in galaxy.json." << std::endl;
    running = false;
    cvJob.notify_all();
    joinSaveCronThread();
    saveGalaxyToFile();
    exit(0);
}
#else
void GalaxyServer::signalHandler(int signal) {
    std::cout << "Information: Shutting down the server and saving data in galaxy.json. Signal: " << signal << std::endl;
    running = false;
    cvJob.notify_all();
    instance->joinSaveCronThread();
    saveGalaxyToFile();
    exit(signal);
}
#endif