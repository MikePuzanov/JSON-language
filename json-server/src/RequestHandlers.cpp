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
#include "GalaxyServer.h"
#include "RequestHandlers.h"

using namespace std;

crow::response RequestHandler::GetHandler(const crow::request& req) {
    try {
            cout << endl << "Information: Starting Get" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "Information: Request: " + jsonRequest.dump() << endl;

            if (jsonRequest.empty()) {
                return crow::response{200, galaxyServer.getGalaxy().dump()};
            }

            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                cout << "Information: Go in get fucntion" << endl;
                json result = galaxyServer.processGet(jsonRequest);
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
}

crow::response RequestHandler::AddHandler(const crow::request& req) {
    try {
            cout << endl << "Information: Starting Add" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "Information: Request: " + jsonRequest.dump() << endl;
            
            if (jsonRequest.is_array()) {
                if (jsonRequest.size() != 2) {
                    throw InvalidJSONFormatException("Error: Not valid request body. Request body should be array with two elements. Request: " + jsonRequest.dump());
                }

                cout << "Information: Go to add function" << endl;
                galaxyServer.processAdd(jsonRequest[0], jsonRequest[1]);           
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
}

crow::response RequestHandler::DeleteHandler(const crow::request& req) {
    try {
            cout << endl << "Information: Starting Delete" << endl;
            auto jsonRequest = json::parse(req.body);
            cout << "Information: Request: " + jsonRequest.dump() << endl;
            
            if (jsonRequest.is_array() || jsonRequest.is_object()) {
                cout << "Information: Go in delete function" << endl;
                galaxyServer.processDelete(jsonRequest);
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
}
