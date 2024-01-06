#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <httplib.h>
#include <json.hpp>

TEST_CASE("Add JSON Request", "[add_json]") {
    // Запускаем ваш сервер (предполагая, что он уже скомпилирован и исполняем)
    // В реальном тестовом окружении вам, возможно, придется запустить сервер в отдельном процессе.

    // Создаем HTTP-клиента для взаимодействия с сервером
    httplib::Client client("localhost", 18080);

    // Формируем JSON-запрос
    nlohmann::json requestBody = {
        {"key", "test_key"},
        {"value", 42.0}
    };

    // Отправляем запрос на сервер
    auto response = client.Post("/add_json", requestBody.dump(), "application/json");

    // Проверяем, что ответ сервера содержит ожидаемый JSON
    REQUIRE(response);
    REQUIRE(response->status == 200);

    nlohmann::json responseBody = nlohmann::json::parse(response->body);
    REQUIRE(responseBody["status"] == "success");
}

TEST_CASE("Get JSON Request", "[get_json]") {
    // То же самое: запускаем сервер и создаем HTTP-клиента

    // Формируем JSON-запрос для добавления данных
    nlohmann::json addRequest = {
        {"key", "test_key"},
        {"value", 42.0}
    };

    // Отправляем запрос на сервер
    httplib::Client addClient("localhost", 18080);
    auto addResponse = addClient.Post("/add_json", addRequest.dump(), "application/json");
    REQUIRE(addResponse);
    REQUIRE(addResponse->status == 200);

    // Теперь формируем запрос на получение данных
    nlohmann::json getRequest = {
        {"key", "test_key"}
    };

    // Отправляем запрос на сервер
    httplib::Client getClient("localhost", 18080);
    auto getResponse = getClient.Post("/get", getRequest.dump(), "application/json");

    // Проверяем, что ответ сервера содержит ожидаемый JSON
    REQUIRE(getResponse);
    REQUIRE(getResponse->status == 200);

    nlohmann::json getResponseBody = nlohmann::json::parse(getResponse->body);
    REQUIRE(getResponseBody["status"] == "success");
    REQUIRE(getResponseBody["value"] == 42.0);
}
