#pragma once

#include <string>
#include <nlohmann/json.hpp>

#include "httplib.h"
#include <regex>
#include <stdexcept>

using namespace std;
using namespace nlohmann;

class JsonLanguage {
public:
    // Receiving by sequence
    json get(const json &query);
    // Recording by sequence
    void add(const json& command);
    // Delete by sequence
    void remove(const json& command);

private:
    // Local galaxy
    json galaxy;
    // Local get of information from galaxy
    json processGet(const json& query);
    // Local recording of information in galaxy
    void processAdd(const json& command, const json& result);
    // Local deleting data deom galaxy
    void processDelete(const json& query);
    // Check string on format URL
    bool isURL(const string &str);
    // Validate response from server
    void validateResponse(string fullUrl, httplib::Result &response);
};

#ifndef JSONLANGUAGE_H
#define JSONLANGUAGE_H

// Invalid format combination exception
class InvalidCombinationException : public runtime_error {
public:
    explicit InvalidCombinationException(const string& msg, json combination) : runtime_error(msg) {
        this->combination = combination;
    }

    string getCombination() { return combination.dump(); }
private:
    json combination;
};

// (Server) Invalid format combination exception
class InvalidCombinationServerException : public runtime_error {
public:
    explicit InvalidCombinationServerException(const string& msg, string response, string url) : runtime_error(msg) {
        this->response = response;
        this->url = url;
    }

    string getResponse() { return response; }
    string getServerUrl() { return url; }
private:
    string response;
    string url;
};

// Not found object from combination in galaxy
class NotFoundDataException : public runtime_error {
public:
    explicit NotFoundDataException(const string& msg, string dataNotFound) : runtime_error(msg) {
        this->dataNotFound = dataNotFound;
    }
    
    string getDataNotFound() { return dataNotFound; }
private:
    string dataNotFound;
};

// (Server) Not found object from combination in galaxy
class NotFoundDataServerException : public runtime_error {
public:
    explicit NotFoundDataServerException(const string& msg, string response, string url) : runtime_error(msg) {
        this->response = response;
        this->url = url;
    }
    
    string getResponse() { return response; }
    string getServerUrl() { return url; }
private:
    string response;
    string url;
};

// Not connection with server
class FailedConnectionException : public runtime_error {
public:
    explicit FailedConnectionException(const string& msg, string serverUrl) : runtime_error(msg) {
        this->serverUrl = serverUrl;
    }

    string getServerUrl() { return serverUrl; }
private:
    string serverUrl;
};

// Not Valid format JSON
class InvalidJSONFormatException : public runtime_error {
public:
    InvalidJSONFormatException(const string& msg, json invalidJson) : runtime_error(msg) {
        this->invalidJson = invalidJson;
    }

    string getInvalidJson() { return invalidJson.dump(); }
private:
    json invalidJson;
};

// (Server) Not Valid format JSON
class InvalidJSONFormatServerException : public runtime_error {
public:
    InvalidJSONFormatServerException(const string& msg, string response, string url) : runtime_error(msg) {
        this->response = response;
        this->url = url;
    }

    string getResponse() { return response; }
    string getServerUrl() { return url; }
private:
    string response;
    string url;
};

// Unexpected server error
class ServerException : public runtime_error {
public:
    ServerException(const string& msg, string serverUrl) : runtime_error(msg) {
        this->serverUrl = serverUrl;
    }

    string getServerUrl() { return serverUrl; }
private:
    string serverUrl;
};

// Out of range array
class IndexException : public runtime_error {
public:
    explicit IndexException(const string& msg, string arrayName, int lenght) : runtime_error(msg) {
        this->arrayName = arrayName;
        this->lenght = lenght;
    }

    string getArrayName() { return arrayName; }
    int getArrayLenght() { return lenght; }
private:
    string arrayName;
    int lenght;
};

// (Server) Out of range array
class IndexServerException : public runtime_error {
public:
    explicit IndexServerException(const string& msg, string response, string serverUrl) : runtime_error(msg) {
        this->response = response;
        this->serverUrl = serverUrl;
    }

    string getResponse() { return response; }
    string getServerUrl() { return serverUrl; }
private:
    string serverUrl;
    string response;
};

// This object is not array
class IsNotArrayException : public runtime_error {
public:
    explicit IsNotArrayException(const string& msg, string objectName) : runtime_error(msg) {
        this->objectName = objectName;
    }

    string getObjectName() { return objectName; }
private:
    string objectName;
};

// (Server) This object is not array
class IsNotArrayServerException : public runtime_error {
public:
    explicit IsNotArrayServerException(const string& msg, string response, string serverUrl) : runtime_error(msg) {
        this->response = response;
        this->serverUrl = serverUrl;
    }

    string getResponse() { return response; }
    string getServerUrl() { return serverUrl; }
private:
    string serverUrl;
    string response;
};

#endif // JSONLANGUAGE_H
