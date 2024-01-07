#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <httplib.h>
#include <nlohmann/json.hpp>

TEST_CASE("Add JSON Request", "[add_json]") {
    httplib::Client client("localhost", 18080);

    nlohmann::json requestBody = {
        {"key", "test_key"},
        {"value", 42.0}
    };

    auto response = client.Post("/add", requestBody.dump(), "application/json");

    REQUIRE(response);
    REQUIRE(response->status == 200);

    nlohmann::json responseBody = nlohmann::json::parse(response->body);
    REQUIRE(responseBody["status"] == "success");
}

TEST_CASE("Get JSON Request", "[get]") {
    nlohmann::json addRequest = {
        {"key", "test_key"},
        {"value", 42.0}
    };

    httplib::Client addClient("localhost", 18080);
    auto addResponse = addClient.Post("/add", addRequest.dump(), "application/json");
    REQUIRE(addResponse);
    REQUIRE(addResponse->status == 200);

    nlohmann::json getRequest = {
        {"key", "test_key"}
    };

    httplib::Client getClient("localhost", 18080);
    auto getResponse = getClient.Post("/get", getRequest.dump(), "application/json");

    REQUIRE(getResponse);
    REQUIRE(getResponse->status == 200);

    nlohmann::json getResponseBody = nlohmann::json::parse(getResponse->body);
    REQUIRE(getResponseBody == 42.0);
}

TEST_CASE("Get JSON Request with Array", "[get_array]") {
    nlohmann::json addArrayRequest = {
        {"key", "test_array"},
        {"value", {1, 2, 3, 4, 5}}
    };

    httplib::Client addArrayClient("localhost", 18080);
    auto addArrayResponse = addArrayClient.Post("/add", addArrayRequest.dump(), "application/json");
    REQUIRE(addArrayResponse);
    REQUIRE(addArrayResponse->status == 200);

    nlohmann::json getArrayRequest = {
        {"key", "test_array"},
        {"index", 2} 
    };

    httplib::Client getArrayClient("localhost", 18080);
    auto getArrayResponse = getArrayClient.Post("/get", getArrayRequest.dump(), "application/json");

    REQUIRE(getArrayResponse);
    REQUIRE(getArrayResponse->status == 200);

    nlohmann::json getArrayResponseBody = nlohmann::json::parse(getArrayResponse->body);
    REQUIRE(getArrayResponseBody == 3); 
}

TEST_CASE("Get JSON Request with Nonexistent Key", "[get_nonexistent_key]") {
    nlohmann::json getNonexistentRequest = {
        {"key", "nonexistent_key"}
    };

    httplib::Client getNonexistentClient("localhost", 18080);
    auto getNonexistentResponse = getNonexistentClient.Post("/get", getNonexistentRequest.dump(), "application/json");

    REQUIRE(getNonexistentResponse);
    REQUIRE(getNonexistentResponse->status == 404);

    nlohmann::json getNonexistentResponseBody = nlohmann::json::parse(getNonexistentResponse->body);
    REQUIRE(getNonexistentResponseBody["status"] == "error");
    REQUIRE(getNonexistentResponseBody["message"] == "Key not found");
}

TEST_CASE("Invalid JSON Format Request", "[invalid_json_format]") {
    // То же самое: запускаем сервер и создаем HTTP-клиента

    // Отправляем запрос с некорректным JSON форматом
    httplib::Client invalidJsonClient("localhost", 18080);
    auto invalidJsonResponse = invalidJsonClient.Post("/add", "{invalid_json_format}", "application/json");

    // Проверяем, что ответ сервера содержит ожидаемый код ошибки 400
    REQUIRE(invalidJsonResponse);
    REQUIRE(invalidJsonResponse->status == 400);
}

TEST_CASE("Nonexistent Key Request", "[nonexistent_key]") {
    // То же самое: запускаем сервер и создаем HTTP-клиента

    // Формируем JSON-запрос для попытки получения данных по несуществующему ключу
    nlohmann::json nonexistentKeyRequest = {
        {"key", "nonexistent_key"}
    };

    // Отправляем запрос на сервер
    httplib::Client nonexistentKeyClient("localhost", 18080);
    auto nonexistentKeyResponse = nonexistentKeyClient.Post("/get", nonexistentKeyRequest.dump(), "application/json");

    // Проверяем, что ответ сервера содержит ожидаемый код ошибки 404
    REQUIRE(nonexistentKeyResponse);
    REQUIRE(nonexistentKeyResponse->status == 404);
}
