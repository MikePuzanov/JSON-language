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

// Ôóíêöèÿ äëÿ ñîõðàíåíèÿ äàííûõ galaxy â ôàéë
void saveGalaxyToFile(const json& galaxy) {
    ofstream file(galaxyFileName);
    if (file.is_open()) {
        lock_guard<mutex> lock(fileMutex);
        file << galaxy.dump(4); // Çàïèñûâàåì îòôîðìàòèðîâàííûé JSON ñ îòñòóïàìè â 4 ïðîáåëà
        file.close();
        cout << "Äàííûå ñîõðàíåíû â ôàéë " << galaxyFileName << endl;
    } else {
        cerr << "Îøèáêà ïðè îòêðûòèè ôàéëà " << galaxyFileName << " äëÿ çàïèñè." << endl;
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
                    throw IndexException("Âûõîä çà ðàìåðû ìàññèâà. Màññèâ: " + result.dump());
                }
            } else {
                throw IsNotArrayException("Äëÿ âûáîðà â ìàññèâå íóæåí ÷èñëîâîé èíäåêñ. Màññèâ: " + result.dump());
            }
        } else {
            // Åñëè óñëîâèÿ íå âûïîëíèëèñü, âîçâðàùàåì îøèáêó
            throw NotFoundDataException("Íåò òàêîãî ïîëÿ. Ïîëå: " + step.dump());
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
            // Íåêîððåêòíûé ïóòü, âîçâðàùàåì îøèáêó
            cerr << "Error: Íåâåðíàÿ êîìàíäà" << endl;
            throw InvalidJSONFormatException("Íåâåðíûé ôîðìàò JSON. Error: Íåâåðíàÿ êîìàíäà " + step.dump());
        }
    }

    *currentLevel = result;
}

// Îáðàáîò÷èê ñèãíàëà çàâåðøåíèÿ
// void signalHandler(int signal) {
//      cout << "Âûçâàí ñèãíàë çàâåðøåíèÿ. Ñîõðàíåíèå äàííûõ è çàâåðøåíèå ïðîãðàììû." << endl;
//     if (signal == SIGINT || signal == SIGTERM) {
//         // Äåéñòâèÿ ïðè ïîëó÷åíèè ñèãíàëà çàâåðøåíèÿ
//         cout << "Ïîëó÷åí ñèãíàë çàâåðøåíèÿ. Ñîõðàíåíèå äàííûõ è çàâåðøåíèå ïðîãðàììû." << endl;
//         // Ñîõðàíÿåì äàííûå ïåðåä çàâåðøåíèåì
//         saveGalaxyToFile(galaxy);
//         exit(signal);
//     }
// }

#ifdef _WIN32
#include <Windows.h>

// Îáðàáîò÷èê ñîáûòèé êîíñîëè äëÿ Windows
BOOL WINAPI ConsoleHandler(DWORD signal) {
    switch (signal) {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        case CTRL_LOGOFF_EVENT:
            // Îáðàáîòêà ñîáûòèÿ
            cout << "Âûçâàí îáðàáîò÷èê ñèãíàëà " << signal << endl;
            saveGalaxyToFile(galaxy);
            exit(signal);
        default:
            return FALSE;
    }
}
#else
#include <unistd.h>

// Îáðàáîò÷èê ñèãíàëîâ äëÿ Linux
void signalHandler(int signal) {
    // Îáðàáîòêà ñèãíàëà
    cout << "Âûçâàí îáðàáîò÷èê ñèãíàëà " << signal << endl;
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

    galaxy = { { "afsafsdfsdf", "afwa14214214214fwar"} };

    saveGalaxyToFile(galaxy);

    #ifdef _WIN32
    // Ðåãèñòðàöèÿ îáðàáîò÷èêà êîíñîëè äëÿ Windows
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        cerr << "Îøèáêà ïðè ðåãèñòðàöèè îáðàáîò÷èêà êîíñîëè" << endl;
        return EXIT_FAILURE;
    }
#else
    // Ðåãèñòðàöèÿ îáðàáîò÷èêà ñèãíàëîâ äëÿ Linux
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGKILL, signalHandler);
    signal(SIGSTOP, signalHandler);
#endif

    crow::SimpleApp app;

    // Îáðàáîò÷èê GET çàïðîñà ïî ïóòè /get
    CROW_ROUTE(app, "/get").methods("POST"_method)([](const crow::request& req) {
        try {
            cout << endl << endl << "Ñòàðò çàïðîñà Get" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "Òåëî çàïðîñà " + jsonRequest.dump() << endl;


            if (jsonRequest.empty()) {
                return crow::response{200, galaxy.dump()};
            }

            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                lock_guard<mutex> lock(galaxyMutex);

                cout << "Ïåðåõîä â ôóíêöèþ" << endl;
                json result = processGet(jsonRequest);
                cout << "Êîíåö ôóíêöèè" << endl;

                if (result.is_object() && result.find("status") != result.end() && result["status"] == "error") {    
                    return crow::response{404, result.dump()};
                } else {
                    return crow::response{200, result.dump()};
                }
            } else {
                // Åñëè çàïðîñ íå ÿâëÿåòñÿ ìàññèâîì èëè îáúåêòîì, âîçâðàùàåì îøèáêó
                return crow::response{400, "Íåïðàâèëüíûé ôîðìàò JSON"};
            }
        } catch (const IndexException& e) {
            cout << "Ïîéìàëè îøèáêó IndexException." << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const IsNotArrayException& e) {
            cout << "Ïîéìàëè îøèáêó IsNotArrayException." << endl;
            cerr <<  e.what();
            return crow::response{400, e.what()};
        } catch (const NotFoundDataException& e) {
            cout << "Ïîéìàëè îøèáêó NotFoundDataException." << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "Ïîéìàëè îøèáêó." << endl;
            cerr <<  e.what();
            // Åñëè ïðîèçîøëà îøèáêà ïðè ïàðñèíãå JSON, âîçâðàùàåì îøèáêó
            return crow::response{400, e.what()};//"Íåïðàâèëüíûé ôîðìàò JSON"};
        }
    });

    // Ýíäïîèíò äëÿ ñîçäàíèÿ/îáíîâëåíèÿ îáúåêòà JSON ïî óêàçàòåëþ
    CROW_ROUTE(app, "/add").methods("POST"_method)([](const crow::request& req) {
        try {
            auto jsonRequest = json::parse(req.body);
            cout << "Òåëî çàïðîñà " + jsonRequest.dump() << endl;
            
            if (jsonRequest.is_array()) {
                if (jsonRequest.size() != 2) {
                    throw InvalidJSONFormatException("Òåëî çàïðîñà äîëæíî ñîäåðæàòü ìàññèâ èç 2 ýëåìåíòîâ. Òåëî = " + jsonRequest.dump());
                }
                lock_guard<mutex> lock(galaxyMutex);
                cout << "Ïåðåõîä â ôóíêöèþ çàïèñè" << endl;
                processAdd(jsonRequest[0], jsonRequest[1]);           
                cout << "Êîíåö ôóíêöèè çàïèñè" << endl;
                return crow::response{200, "Success"};
            } else {
                return crow::response{400, "Íåïðàâèëüíûé ôîðìàò JSON"};
            }
        } catch (const InvalidJSONFormatException& e) {
            cout << "Ïîéìàëè îøèáêó InvalidJSONFormatException." << endl;
            cerr <<  e.what();
            return crow::response{404, e.what()};
        } catch (const exception& e) {
            cout << "Ïîéìàëè îøèáêó" << endl;
            return crow::response{400, "Íåïðàâèëüíûé ôîðìàò JSON"};
        }
    });  

   // Òàéìåð äëÿ ïåðèîäè÷åñêîãî ñîõðàíåíèÿ äàííûõ galaxy â ôàéë
    auto saveTimer = [&]() {
        while (true) {
            // Ñîõðàíÿåì galaxy â ôàéë "galaxy.json"
            cout << "Ñîõðàíÿåì galaxy â ôàéë galaxy.json ÷åðåç Job";
            saveGalaxyToFile(galaxy);
            // Çàñûïàåì íà 5 ìèíóò
            this_thread::sleep_for(chrono::minutes(5));
        }
    };

    // Çàïóñêàåì òàéìåð â îòäåëüíîì ïîòîêå
    thread saveThread(saveTimer);

    // Çàïóñêàåì ïðèëîæåíèå
    app.bindaddr(host).port(port).multithreaded().run();

    // Äîæèäàåìñÿ çàâåðøåíèÿ ðàáîòû òàéìåðà
    saveThread.join();

    return 0;
}