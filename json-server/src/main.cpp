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
const string galaxyFileName = "galaxy.json";

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

// Функция для сохранения данных galaxy в файл
void saveGalaxyToFile(const json& galaxy) {
    ofstream file(galaxyFileName);
    if (file.is_open()) {
        lock_guard<mutex> lock(fileMutex);
        file << galaxy.dump(4); // Записываем отформатированный JSON с отступами в 4 пробела
        file.close();
        cout << "Данные сохранены в файл " << galaxyFileName << endl;
    } else {
        cerr << "Ошибка при открытии файла " << galaxyFileName << " для записи." << endl;
    }
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
                    throw IndexException("Выход за рамеры массива. Mассив: " + result.dump());
                }
            } else {
                throw IsNotArrayException("Для выбора в массиве нужен числовой индекс. Mассив: " + result.dump());
            }
        } else {
            // Если условия не выполнились, возвращаем ошибку
            throw NotFoundDataException("Нет такого поля. Поле: " + step.dump());
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
            // Некорректный путь, возвращаем ошибку
            cerr << "Error: Неверная команда" << endl;
            throw InvalidJSONFormatException("Неверный формат JSON. Error: Неверная команда " + step.dump());
        }
    }

    *currentLevel = result;
}

#ifdef _WIN32
#include <Windows.h>

// Обработчик событий консоли для Windows
BOOL WINAPI ConsoleHandler(DWORD signal) {
    switch (signal) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_LOGOFF_EVENT:
            // Обработка события
            cout << "Вызван обработчик сигнала " << signal << endl;
            saveGalaxyToFile(galaxy);
            exit(signal);
        default:
            return FALSE;
    }
}
#else
#include <unistd.h>

// Обработчик сигналов для Linux
void signalHandler(int signal) {
    // Обработка сигнала
    cout << "Вызван обработчик сигнала " << signal << endl;
    saveGalaxyToFile(galaxy);
    exit(signal);
}
#endif

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    std::string host = "";
    int port = 0;
    cout << argc;
    if (argc != 3) {
        json config = loadConfig("serverConfig.json");
        host = config["ip"];
        port = config["port"];
    } else {
        host = argv[1];
        port = stoi(argv[2]);
    }

    #ifdef _WIN32
    // Регистрация обработчика консоли для Windows
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "Ошибка при регистрации обработчика консоли" << endl;
        return EXIT_FAILURE;
    }
#else
    // Регистрация обработчика сигналов для Linux
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGSTOP, signalHandler);
#endif

    crow::SimpleApp app;

    // Обработчик GET запроса по пути /get
    CROW_ROUTE(app, "/get").methods("POST"_method)([](const crow::request& req) {
        try {
            cout << endl << endl << "Старт запроса Get" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "Тело запроса " + jsonRequest.dump() << endl;


            if (jsonRequest.empty()) {
                return crow::response{200, galaxy.dump()};
            }

            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                lock_guard<mutex> lock(galaxyMutex);

                cout << "Переход в функцию" << endl;
                json result = processGet(jsonRequest);
                cout << "Конец функции" << endl;

                if (result.is_object() && result.find("status") != result.end() && result["status"] == "error") {    
                    return crow::response{404, result.dump()};
                } else {
                    return crow::response{200, result.dump()};
                }
            } else {
                // Если запрос не является массивом или объектом, возвращаем ошибку
                return crow::response{400, "Неправильный формат JSON"};
            }
        } catch (const IndexException& e) {
            cout << "Поймали ошибку IndexException." << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const IsNotArrayException& e) {
            cout << "Поймали ошибку IsNotArrayException." << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const NotFoundDataException& e) {
            cout << "Поймали ошибку NotFoundDataException." << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "Поймали ошибку." << endl;
            cerr <<  e.what();
            // Если произошла ошибка при парсинге JSON, возвращаем ошибку
            return crow::response{400, e.what()};//"Неправильный формат JSON"};
        }
    });

    // Эндпоинт для создания/обновления объекта JSON по указателю
    CROW_ROUTE(app, "/add").methods("POST"_method)([](const crow::request& req) {
        try {
            auto jsonRequest = json::parse(req.body);
            cout << "Тело запроса " + jsonRequest.dump() << endl;
            
            if (jsonRequest.is_array()) {
                if (jsonRequest.size() != 2) {
                    throw InvalidJSONFormatException("Тело запроса должно содержать массив из 2 элементов. Тело = " + jsonRequest.dump());
                }
                lock_guard<mutex> lock(galaxyMutex);
                cout << "Переход в функцию записи" << endl;
                processAdd(jsonRequest[0], jsonRequest[1]);           
                cout << "Конец функции записи" << endl;
                return crow::response{200, "Success"};
            } else {
                return crow::response{400, "Неправильный формат JSON"};
            }
        } catch (const InvalidJSONFormatException& e) {
            cout << "Поймали ошибку InvalidJSONFormatException." << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "Поймали ошибку" << endl;
            return crow::response{400, "Неправильный формат JSON"};
        }
    });  

   // Таймер для периодического сохранения данных galaxy в файл
    auto saveTimer = [&]() {
        while (true) {
            // Сохраняем galaxy в файл "galaxy.json"
            cout << "Сохраняем galaxy в файл galaxy.json через Job";
            saveGalaxyToFile(galaxy);
            // Засыпаем на 5 минут
            this_thread::sleep_for(chrono::minutes(5));
        }
    };

    // Запускаем таймер в отдельном потоке
    thread saveThread(saveTimer);

    // Запускаем приложение
    app.bindaddr(host).port(port).multithreaded().run();

    // Дожидаемся завершения работы таймера
    saveThread.join();

    return 0;
}