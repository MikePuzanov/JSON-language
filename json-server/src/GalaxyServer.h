#pragma once

#include <thread>
#include <nlohmann/json.hpp>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using json = nlohmann::json;
using namespace std;

class GalaxyServer {
public:
    //
    json getGalaxy();
    //
    json loadConfig();
    // load local galaxy from file
    void loadDataFromGalaxyJson();
    // save data from galaxy to file
    static void saveGalaxyToFile();
    // to get data from galaxy
    json processGet(const json& query);
    // to add data in galaxy
    void processAdd(const json& command, const json& result);
    // to delete data from galaxy
    void processDelete(const json& query);
    //
    void setSignal();
    //
    void addSaveCronJob();
    //
    void joinSaveCronThread();
private:
    thread cronThread;
    
#ifdef _WIN32
    static BOOL WINAPI ConsoleHandler(DWORD signal);
#else
    void signalHandler(int signal);
#endif
    static void shutDownWindows();
};