#ifndef GALAXY_SERVER_H
#define GALAXY_SERVER_H

#pragma once

#include <thread>
#include <nlohmann/json.hpp>
#include <atomic>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using json = nlohmann::json;
using namespace std;

class GalaxyServer {
public:
    public:
    GalaxyServer();
    ~GalaxyServer();

    // get galaxy
    json getGalaxy();
    // load data from config
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
    // set signal for OS
    void setSignal();
    // add cron job for saving galaxy
    void addSaveCronJob();
    // register job
    static void joinSaveCronThread();
private:
    static std::atomic<bool> running;
    static mutex jobMutex;
    static condition_variable cvJob;
    static thread cronThread;
    static int saveCronJobTime;
    static GalaxyServer* instance;
    
#ifdef _WIN32
    static BOOL WINAPI consoleHandler(DWORD signal);
#else
    static void signalHandler(int signal);
#endif
    static void shutDownWindows();
};

#endif // GALAXY_SERVER_H