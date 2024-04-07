#include <crow.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <fstream>
#include <iostream>
#include "MyExceptions.h"
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
const string galaxyFileNameUbuntu = "/usr/bin/jsonServer/galaxy.json";
const string configFileNameUbuntu = "/usr/bin/jsonServer/serverConfig.json";
// Const Ubuntu for .deb pack 
//const string galaxyFileNameUbuntu = "/usr/bin/jsonServer/galaxy.json";
//const string configFileNameUbuntu = "/usr/bin/jsonServer/serverConfig.json";


json loadConfig() {
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

void loadDataFromGalaxyJson() {
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
        cerr << e.what(); 
    }
}

// ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ galaxy ˜ ˜˜˜˜
void saveGalaxyToFile(const json& galaxy) {
#ifdef _WIN32
        ofstream file(galaxyFileNameWindows);
#else
        ofstream file(galaxyFileNameUbuntu);
#endif
    
    if (file.is_open()) {
        lock_guard<mutex> lock(fileMutex);
        file << galaxy.dump(4); // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜˜˜˜ JSON ˜ ˜˜˜˜˜˜˜˜˜ ˜ 4 ˜˜˜˜˜˜˜
        file.close();
        cout << endl << "˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜ ˜ ˜˜˜˜ " << galaxyFileNameWindows << endl;
    } else {
        cerr << "˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜ " << galaxyFileNameWindows << " ˜˜˜ ˜˜˜˜˜˜." << endl;
    }
}

json processGet(const json& query) {
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
                    throw IndexException("˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜. M˜˜˜˜˜: " + result.dump());
                }
            } else {
                throw IsNotArrayException("˜˜˜ ˜˜˜˜˜˜ ˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜. M˜˜˜˜˜: " + result.dump());
            }
        } else {
            // ˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜˜˜˜, ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
            throw NotFoundDataException("˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜. ˜˜˜˜: " + step.dump());
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
            // ˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜, ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
            cerr << "Error: ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜" << endl;
            throw InvalidJSONFormatException("˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ JSON. Error: ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ " + step.dump());
        }
    }

    *currentLevel = result;
}

#ifdef _WIN32
#include <Windows.h>

// ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜ Windows
BOOL WINAPI ConsoleHandler(DWORD signal) {
    switch (signal) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_LOGOFF_EVENT:
            // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜
            cout << "˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ " << signal << endl;
            saveGalaxyToFile(galaxy);
            exit(signal);
        default:
            return FALSE;
    }
}
#else
#include <unistd.h>

// ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜ Linux
void signalHandler(int signal) {
    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜
    cout << "˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ " << signal << endl;
    saveGalaxyToFile(galaxy);
    exit(signal);
}
#endif

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    std::string host = "";
    int port = 0;

    if (argc != 3) {
        json config = loadConfig();
        host = config["ip"];
        port = config["port"];
    } else {
        host = argv[1];
        port = stoi(argv[2]);
    }

    loadDataFromGalaxyJson();

    #ifdef _WIN32
    // ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜ Windows
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜" << endl;
        return EXIT_FAILURE;
    }
#else
    // ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜ Linux
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGSTOP, signalHandler);
#endif

    crow::SimpleApp app;

    // ˜˜˜˜˜˜˜˜˜˜ GET ˜˜˜˜˜˜˜ ˜˜ ˜˜˜˜ /get
    CROW_ROUTE(app, "/get").methods("POST"_method)([](const crow::request& req) {
        try {
            cout << endl << "˜˜˜˜˜ ˜˜˜˜˜˜˜ Get" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "˜˜˜˜ ˜˜˜˜˜˜˜ " + jsonRequest.dump() << endl;


            if (jsonRequest.empty()) {
                return crow::response{200, galaxy.dump()};
            }

            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                lock_guard<mutex> lock(galaxyMutex);

                cout << "˜˜˜˜˜˜˜ ˜ ˜˜˜˜˜˜˜" << endl;
                json result = processGet(jsonRequest);
                cout << "˜˜˜˜˜ ˜˜˜˜˜˜˜" << endl;

                if (result.is_object() && result.find("status") != result.end() && result["status"] == "error") {    
                    return crow::response{404, result.dump()};
                } else {
                    return crow::response{200, result.dump()};
                }
            } else {
                // ˜˜˜˜ ˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜, ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
                return crow::response{400, "˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ JSON"};
            }
        } catch (const IndexException& e) {
            cout << "˜˜˜˜˜˜˜ ˜˜˜˜˜˜ IndexException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const IsNotArrayException& e) {
            cout << "˜˜˜˜˜˜˜ ˜˜˜˜˜˜ IsNotArrayException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const NotFoundDataException& e) {
            cout << "˜˜˜˜˜˜˜ ˜˜˜˜˜˜ NotFoundDataException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "˜˜˜˜˜˜˜ ˜˜˜˜˜˜. " << e.what() << endl;
            cerr <<  e.what();
            // ˜˜˜˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜ JSON, ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
            return crow::response{400, e.what()};//"˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ JSON"};
        }
    });

    // ˜˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜/˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ JSON ˜˜ ˜˜˜˜˜˜˜˜˜
    CROW_ROUTE(app, "/add").methods("POST"_method)([](const crow::request& req) {
        try {
            cout << endl << "˜˜˜˜˜ ˜˜˜˜˜˜˜ Add" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "˜˜˜˜ ˜˜˜˜˜˜˜ " + jsonRequest.dump() << endl;
            
            if (jsonRequest.is_array()) {
                if (jsonRequest.size() != 2) {
                    throw InvalidJSONFormatException("˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜ 2 ˜˜˜˜˜˜˜˜˜. ˜˜˜˜ = " + jsonRequest.dump());
                }
                lock_guard<mutex> lock(galaxyMutex);
                cout << "˜˜˜˜˜˜˜ ˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜" << endl;
                processAdd(jsonRequest[0], jsonRequest[1]);           
                cout << "˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜˜˜˜" << endl;
                return crow::response{200, "Success"};
            } else {
                return crow::response{400, "˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ JSON"};
            }
        } catch (const InvalidJSONFormatException& e) {
            cout << "˜˜˜˜˜˜˜ ˜˜˜˜˜˜ InvalidJSONFormatException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "˜˜˜˜˜˜˜ ˜˜˜˜˜˜. " << e.what() << endl;
            return crow::response{400, "˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ JSON"};
        }
    });  

   // ˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ galaxy ˜ ˜˜˜˜
    auto saveTimer = [&]() {
        while (true) {
            this_thread::sleep_for(chrono::minutes(5));
            // ˜˜˜˜˜˜˜˜˜ galaxy ˜ ˜˜˜˜ "galaxy.json"
            cout << "˜˜˜˜˜˜˜˜˜ galaxy ˜ ˜˜˜˜ galaxy.json ˜˜˜˜˜ Job" << endl;
            saveGalaxyToFile(galaxy);
            // ˜˜˜˜˜˜˜˜ ˜˜ 5 ˜˜˜˜˜
        }
    };

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜ ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
    thread saveThread(saveTimer);

    // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜
    app.bindaddr(host).port(port).multithreaded().run();

    // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜˜˜˜˜
    saveThread.join();

    return 0;
}