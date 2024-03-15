#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>

TEST_CASE("Add JSON Request", "[add_json]") {
    httplib::Client client("localhost", 18080);

    nlohmann::json requestBody = {{"one"}, {"two", 3}};

    auto response = client.Post("/add", requestBody.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 200);
}

TEST_CASE("Get JSON Request", "[get]") {
    httplib::Client client("localhost", 18080);

    nlohmann::json addRequest = {{"test_key"}, 42.0};
    client.Post("/add", addRequest.dump(), "application/json");

    nlohmann::json getRequest = {"test_key"};

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 200);

    nlohmann::json responseBody = nlohmann::json::parse(response->body);
    REQUIRE(responseBody == 42.0);
}

TEST_CASE("Get JSON Request with Nonexistent Key", "[get_nonexistent_key]") {
    httplib::Client client("localhost", 18080);

    nlohmann::json getRequest = {
        {"nonexistent_key"}
    };

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 404);
}

TEST_CASE("Invalid JSON Format Request", "[invalid_json_format]") {
    httplib::Client client("localhost", 18080);

    auto response = client.Post("/add", "{invalid_json_format}", "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 400);
}

TEST_CASE("Nonexistent Key Request", "[nonexistent_key]") {
    httplib::Client client("localhost", 18080);

    nlohmann::json getRequest = {
        {"nonexistent_key"}
    };

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 404);
}

TEST_CASE("Index Exception", "[index_exception]") {
    httplib::Client client("localhost", 18080);
    nlohmann::json addRequest = {{7, 2, 5, 0}};
    client.Post("/add", addRequest.dump(), "application/json");

    nlohmann::json getRequest = {10};

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 404);
}

TEST_CASE("No array Exception", "[no_array_exception]") {
    httplib::Client client("localhost", 18080);
    nlohmann::json addRequest = {{}, {1, "zs", {{"v","ret"}, {"hl",1}}}};
    client.Post("/add", addRequest.dump(), "application/json");

    nlohmann::json getRequest = {"v"};

    auto response = client.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 400);
}