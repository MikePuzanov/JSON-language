// #include <catch2/catch_all.hpp>// подключаем Catch2
// #include "json-language.h"  // подключаем вашу библиотеку

// const std::string url = "http://0.0.0.0:18080/";

// TEST_CASE("Successful GET request") {
//     MyLibrary library;

//     // Создаем JSON-команду для GET-запроса
//     nlohmann::json getRequest = {"http://192.168.0.1", "get"};

//     // Запускаем GET-запрос
//     nlohmann::json result = library.get(getRequest);

//     // Проверяем, что результат не пустой
//     REQUIRE(!result.empty());

//     // TODO: Добавьте дополнительные проверки, соответствующие вашей логике
//     // Например, проверьте, что полученный JSON соответствует ожидаемым данным
// }