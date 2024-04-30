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
const string galaxyFileNameUbuntu = "galaxy.json";
const string configFileNameUbuntu = "serverConfig.json";
// Const Ubuntu for .deb pack 
//const string galaxyFileNameUbuntu = "/usr/bin/jsonServer/galaxy.json";
//const string configFileNameUbuntu = "/usr/bin/jsonServer/serverConfig.json";

// load coniguration from file
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

// load local galaxy from file
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
        cerr << "Error: " << e.what(); 
    }
}

// save data from galaxy to file
void saveGalaxyToFile(const json& galaxy) {
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
            cerr << "Error: Invalid JSON format" << endl;
            throw InvalidJSONFormatException("Error: Invalid JSON format. Key: " + step.dump());
        }
    }

    *currentLevel = result;
}

// to delete data from galaxy
void processDelete(const json& query) {
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

#ifdef _WIN32
#include <Windows.h>

// signal for Windows
BOOL WINAPI ConsoleHandler(DWORD signal) {
    switch (signal) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_LOGOFF_EVENT:
            cout << "Information: Shutting down the server and saving data in galaxy.json. Signal: " << signal << endl;
            saveGalaxyToFile(galaxy);
            exit(signal);
        default:
            return FALSE;
    }
}
#else
#include <unistd.h>

// signal for Linux
void signalHandler(int signal) {
    cout << "Information: Shutting down the server and saving data in galaxy.json. Signal: " << signal << endl;
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
    // setup signal for Windows
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "Error: with setup signal habdler" << endl;
        return EXIT_FAILURE;
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

    crow::SimpleApp app;

    // POST /get
    CROW_ROUTE(app, "/get").methods("POST"_method)([](const crow::request& req) {
        try {
            cout << endl << "Information: Starting Get" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "Information: Request: " + jsonRequest.dump() << endl;

            if (jsonRequest.empty()) {
                return crow::response{200, galaxy.dump()};
            }

            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                lock_guard<mutex> lock(galaxyMutex);

                cout << "Information: Go in get fucntion" << endl;
                json result = processGet(jsonRequest);
                cout << "Information: Go out from get fucntion" << endl;

                if (result.is_object() && result.find("status") != result.end() && result["status"] == "error") {    
                    return crow::response{404, result.dump()};
                } else {
                    return crow::response{200, result.dump()};
                }
            } else {
                return crow::response{400, "Invalid format JSON"};
            }
        } catch (const IndexException& e) {
            cout << "Information: Catch IndexException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const IsNotArrayException& e) {
            cout << "Information: Catch IsNotArrayException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const NotFoundDataException& e) {
            cout << "Information: Catch NotFoundDataException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "Information: Catch unknown exception. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        }
    });

    // POST /add
    CROW_ROUTE(app, "/add").methods("POST"_method)([](const crow::request& req) {
        try {
            cout << endl << "Information: Starting Add" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "Information: Request: " + jsonRequest.dump() << endl;
            
            if (jsonRequest.is_array()) {
                if (jsonRequest.size() != 2) {
                    throw InvalidJSONFormatException("Error: Not valid request body. Request body should be array with two elements. Request: " + jsonRequest.dump());
                }
                lock_guard<mutex> lock(galaxyMutex);
                cout << "Information: Go to add function" << endl;
                processAdd(jsonRequest[0], jsonRequest[1]);           
                cout << "Information: Go out from add function" << endl;
                
                return crow::response{200, "Success"};
            } else {
                return crow::response{400, "Invalid JSON format"};
            }
        } catch (const InvalidJSONFormatException& e) {
            cout << "Information: Catch InvalidJSONFormatException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "Information: Catch unknown" << e.what() << endl;
            return crow::response{400, "Invalid JSON format"};
        }
    });

    // Delete /delete
    CROW_ROUTE(app, "/delete").methods("DELETE"_method)([](const crow::request& req) {
        try {
            cout << endl << "Information: Starting Delete" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "Information: Request: " + jsonRequest.dump() << endl;
            
            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                lock_guard<mutex> lock(galaxyMutex);

                cout << "Information: Go in delete function" << endl;
                processDelete(jsonRequest);
                cout << "Information: Go out from delete function" << endl;
                
                return crow::response{200, "Success"};
            } else {
                return crow::response{400, "Invalid format JSON"};
            }
        } catch (const IndexException& e) {
            cout << "Information: Catch IndexException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const IsNotArrayException& e) {
            cout << "Information: Catch IsNotArrayException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const NotFoundDataException& e) {
            cout << "Information: Catch NotFoundDataException. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "Information: Catch unknown exception. " << e.what() << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        }
    });

   // cron job for saving galaxy in file with 5 min timer
    auto saveTimer = [&]() {
        while (true) {
            this_thread::sleep_for(chrono::minutes(5));
            cout << "Information: Saving galaxy in galaxy.json from Job" << endl;
            saveGalaxyToFile(galaxy);
        }
    };

    thread saveThread(saveTimer);

    app.bindaddr(host).port(port).multithreaded().run();

    saveThread.join();

    return 0;
}