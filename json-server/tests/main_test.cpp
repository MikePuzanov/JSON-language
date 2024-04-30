#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>

using namespace std;

const string url = "localhost";
const int port = 4000;
httplib::Client client(url, port);

TEST_CASE("Add JSON Request", "[add_json]") {
    nlohmann::json requestBody = {{"one"}, {"two", 3}};

    auto response = client.Post("/add", requestBody.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 200);
    nlohmann::json request = {};
    client.Delete("/delete", request.dump(), "application/json");
}

TEST_CASE("Get JSON Request", "[get]") {
    nlohmann::json addRequest = {{"test_key"}, 42.0};
    client.Post("/add", addRequest.dump(), "application/json");

    nlohmann::json getRequest = {"test_key"};

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 200);

    nlohmann::json responseBody = nlohmann::json::parse(response->body);
    REQUIRE(responseBody == 42.0);
    nlohmann::json request = {};
    client.Delete("/delete", request.dump(), "application/json");
}

TEST_CASE("Delete JSON Request", "[delete]") {
    nlohmann::json addRequest = nlohmann::json::array();
    addRequest.push_back("data");
    addRequest.push_back({
        {"people", {
            {"name", "mike"},
            {"age", "10"}
        }},
        {"address", {
            {"city", "Bgd"},
            {"street", "Ledonr"}
        }}
    });
    client.Post("/add", addRequest.dump(), "application/json");

    nlohmann::json deleteRequest = {"data", "people", "name"};
    client.Delete("/delete", deleteRequest.dump(), "application/json");

    nlohmann::json getRequest = {"data"};
    auto response = client.Post("/get", getRequest.dump(), "application/json");

    nlohmann::json expectedResult = {
            {"people", {
                {"age", "10"}
            }},
            {"address", {
                {"city", "Bgd"},
                {"street", "Ledonr"}
            }}
    };

    REQUIRE(response);
    REQUIRE(response->status == 200);

    nlohmann::json responseBody = nlohmann::json::parse(response->body);
    REQUIRE(responseBody == expectedResult);
    nlohmann::json request = {};
    client.Delete("/delete", request.dump(), "application/json");
}

TEST_CASE("Get JSON Request with Nonexistent Key", "[get_nonexistent_key]") {
    nlohmann::json getRequest = {
        {"nonexistent_key"}
    };

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 404);
    nlohmann::json request = {};
    client.Delete("/delete", request.dump(), "application/json");
}

TEST_CASE("Invalid JSON Format Request", "[invalid_json_format]") {
    auto response = client.Post("/add", "{invalid_json_format}", "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 400);
    nlohmann::json request = {};
    client.Delete("/delete", request.dump(), "application/json");
}

TEST_CASE("Nonexistent Key Request", "[nonexistent_key]") {
    nlohmann::json getRequest = {
        {"nonexistent_key"}
    };

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 404);
    nlohmann::json request = {};
    client.Delete("/delete", request.dump(), "application/json");
}

TEST_CASE("Index Exception", "[index_exception]") {
    nlohmann::json addRequest = {{7, 2, 5, 0}};
    client.Post("/add", addRequest.dump(), "application/json");

    nlohmann::json getRequest = {10};

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 404);
    nlohmann::json request = {};
    client.Delete("/delete", request.dump(), "application/json");
}

TEST_CASE("No array Exception", "[no_array_exception]") {
    nlohmann::json addRequest = {{}, {1, "zs", {{"v","ret"}, {"hl",1}}}};
    client.Post("/add", addRequest.dump(), "application/json");

    nlohmann::json getRequest = {"v"};

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    addRequest = {{}, {}};
    client.Post("/add", addRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 400);
    nlohmann::json request = {};
    client.Delete("/delete", request.dump(), "application/json");
}