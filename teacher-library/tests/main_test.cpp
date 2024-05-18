#include <catch2/catch_all.hpp>
#include "Teachers.h"
#include <nlohmann/json.hpp>
#include <iostream>

const std::string url = "http://127.0.0.1:4000";
httplib::Client client(url);

TEST_CASE("Server: Add and Get data from Server") {
    Teachers library("galaxy.json");
    nlohmann::json body = nlohmann::json::array();
    body.push_back("data");
    nlohmann::json value = {
        {"people", {
            {"name", "mike"},
            {"age", "10"}
        }},
        {"address", {
            {"city", "Bgd"},
            {"street", "Ledonr"}
        }}
    };
    library.addToServers(body, value, url);

    nlohmann::json getBody = {"data"};
    auto result = library.get(getBody);

    nlohmann::json expectedResult = {
        {"people", {
            {"name", "mike"},
            {"age", "10"}
        }},
        {"address", {
            {"city", "Bgd"},
            {"street", "Ledonr"}
        }}
    };

    REQUIRE(result == expectedResult);
}

TEST_CASE("Remove data") {
    Teachers library("galaxy.json");
    nlohmann::json body = {"data"};
    library.remove(body);

    try {
        library.get(body);

        FAIL("No exception was thrown");
    } catch (const NotFoundDataException& e) {
        // Success
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }

    auto response = client.Post("/get", body.dump(), "application/json");
    REQUIRE(response);
    REQUIRE(response->status != 200);
}