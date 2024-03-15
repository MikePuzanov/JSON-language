#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "MyExceptions.h"
#include "httplib.h"
#include <regex>

class MyLibrary {
public:
    nlohmann::json get(const nlohmann::json& query);
    void add(const nlohmann::json& command);

private:
    nlohmann::json galaxy;  // Локальная галактика
    nlohmann::json processGet(const nlohmann::json& query, const nlohmann::json& current);
    bool isURL(const std::string &str);
    void validateResponse(std::string fullUrl, httplib::Result &response);
    void processAdd(const nlohmann::json& command, const nlohmann::json& result);
};