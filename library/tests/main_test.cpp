#include <catch2/catch_all.hpp>// подключаем Catch2
#include "json-language.h"
#include <nlohmann/json.hpp>
#include <iostream>

const std::string url = "http://127.0.0.1:4000";

TEST_CASE("Invalid JSON in add function") {
    MyLibrary library;

    try {
        nlohmann::json json = { { 1, 2, 3, 4} };
        library.add(json);

        FAIL("No exception was thrown");
    } catch (const InvalidJSONFormatException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}


TEST_CASE("LOCAL TEST: Get JSON Request", "[get]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;

    nlohmann::json addRequest = {{"test_key"}, 42.0};
    library.add(addRequest);
    
    nlohmann::json getRequest = {"test_key"};
    nlohmann::json result = library.get(getRequest);
    
    REQUIRE(result.get<int>() == 42.0);
}

TEST_CASE("LOCAL TEST: Get JSON Request with Nonexistent Key", "[get_nonexistent_key]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;
    nlohmann::json getRequest = {
        {"nonexistent_key"}
    };

    try {
        library.get(getRequest);
        FAIL("No exception was thrown");
    } catch (const NotFoundDataException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("LOCAL TEST: Invalid JSON Format Request", "[invalid_json_format]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;
    nlohmann::json json = {
        {"invalid_json_format"}
    };

    try {
        library.add(json);
        FAIL("No exception was thrown");
    } catch (const InvalidJSONFormatException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("LOCAL TEST: Index Exception", "[index_exception]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;
    
    nlohmann::json json = {{}, {7, 2, 5, 0}};
    library.add(json);

    try {
        json = {10};
        library.get(json);
        FAIL("No exception was thrown");
    } catch (const IndexException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("LOCAL TEST: No array Exception", "[no_array_exception]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;
    
    nlohmann::json json = {{}, {1, "zs", {{"v","ret"}, {"hl",1}}}};
    library.add(json);


    try {
        json = {"v"};
        library.get(json);

        FAIL("No exception was thrown");
    } catch (const IsNotArrayException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: Get JSON Request", "[get]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;

    nlohmann::json addRequest = {{ url, "test_key"}, 42.0};
    library.add(addRequest);
    
    nlohmann::json getRequest = { url, "test_key"};
    nlohmann::json result = library.get(getRequest);
    
    REQUIRE(result.get<int>() == 42.0);
}

TEST_CASE("SERVER TEST: Get JSON Request with Nonexistent Key", "[get_nonexistent_key]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;
    nlohmann::json getRequest = {  url, "nonexistent_key" };

    try {
        library.get(getRequest);
        FAIL("No exception was thrown");
    } catch (const NotFoundDataServerException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: Invalid JSON Format Request", "[invalid_json_format]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;
    nlohmann::json json = { { url, "invalid_json_format" } };

    try {
        library.add(json);
        FAIL("No exception was thrown");
    } catch (const InvalidJSONFormatException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: Index Exception", "[index_exception]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;
    
    nlohmann::json json = {{ url}, {7, 2, 5, 0}};
    library.add(json);

    try {
        json = { url, 10};
        library.get(json);
        FAIL("No exception was thrown");
    } catch (const IndexServerException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: No array Exception", "[no_array_exception]") {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;
    
    nlohmann::json json = {{ url }, {1, "zs", {{"v","ret"}, {"hl",1}}}};
    library.add(json);


    try {
        json = { url, "v"};
        library.get(json);

        FAIL("No exception was thrown");
    } catch (const IsNotArrayServerException& e) {
        // Тип исключения верен, утверждение успешно
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}