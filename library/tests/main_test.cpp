#include <catch2/catch_all.hpp>
#include "JsonLanguage.h"
#include <nlohmann/json.hpp>
#include <iostream>

const std::string url = "http://127.0.0.1:4000";

TEST_CASE("LOCAL TEST: Delete data from galaaxy", "[delete_data]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    library.remove({});
    nlohmann::json addBody = nlohmann::json::array();
    nlohmann::json body = nlohmann::json::array();
    body.push_back("data");
    addBody.push_back(body);
    addBody.push_back({
        {"people", {
            {"name", "mike"},
            {"age", "10"}
        }},
        {"address", {
            {"city", "Bgd"},
            {"street", "Ledonr"}
        }}
    });
    library.add(addBody);
    
    nlohmann::json deleteBody = {"data", "people", "name"};
    library.remove(deleteBody);

    nlohmann::json getBody = {"data"};
    auto result = library.get(getBody);

    nlohmann::json expectedResult = {
            {"people", {
                {"age", "10"}
            }},
            {"address", {
                {"city", "Bgd"},
                {"street", "Ledonr"}
            }}
    };

    REQUIRE(result == expectedResult);
    library.remove({});
}

TEST_CASE("Invalid JSON in add function") {
    JsonLanguage library;

    try {
        nlohmann::json json = { { 1, 2, 3, 4} };
        library.add(json);

        FAIL("No exception was thrown");
    } catch (const InvalidJSONFormatException& e) {
        // Success
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("LOCAL TEST: Get JSON Request", "[get]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;

    nlohmann::json addRequest = {{"test_key"}, 42.0};
    library.add(addRequest);
    
    nlohmann::json getRequest = {"test_key"};
    nlohmann::json result = library.get(getRequest);
    
    REQUIRE(result.get<int>() == 42.0);
}

TEST_CASE("LOCAL TEST: Get JSON Request with Nonexistent Key", "[get_nonexistent_key]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    nlohmann::json getRequest = {
        {"nonexistent_key"}
    };

    try {
        library.get(getRequest);
        FAIL("No exception was thrown");
    } catch (const NotFoundDataException& e) {
        // Success
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("LOCAL TEST: Invalid JSON Format Request", "[invalid_json_format]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    nlohmann::json json = {
        {"invalid_json_format"}
    };

    try {
        library.add(json);
        FAIL("No exception was thrown");
    } catch (const InvalidJSONFormatException& e) {
        // Success
    }
}

TEST_CASE("LOCAL TEST: Index Exception", "[index_exception]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    
    nlohmann::json json = {{}, {7, 2, 5, 0}};
    library.add(json);

    try {
        json = {10};
        library.get(json);
        FAIL("No exception was thrown");
    } catch (const IndexException& e) {
        // Success
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("LOCAL TEST: No array Exception", "[no_array_exception]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    
    nlohmann::json json = {{}, {1, "zs", {{"v","ret"}, {"hl",1}}}};
    library.add(json);


    try {
        json = {"v"};
        library.get(json);

        FAIL("No exception was thrown");
    } catch (const IsNotArrayException& e) {
        // Success
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: Get JSON Request", "[get]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;

    nlohmann::json addRequest = {{ url, "test_key"}, 42.0};
    library.add(addRequest);
    
    nlohmann::json getRequest = { url, "test_key"};
    nlohmann::json result = library.get(getRequest);
    
    REQUIRE(result.get<int>() == 42.0);
}

TEST_CASE("SERVER TEST: Get JSON Request with Nonexistent Key", "[get_nonexistent_key]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    nlohmann::json getRequest = {  url, "nonexistent_key" };

    try {
        library.get(getRequest);
        FAIL("No exception was thrown");
    } catch (const NotFoundDataServerException& e) {
        // Success
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: Invalid JSON Format Request", "[invalid_json_format]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    nlohmann::json json = { { url, "invalid_json_format" } };

    try {
        library.add(json);
        FAIL("No exception was thrown");
    } catch (const InvalidJSONFormatException& e) {
        // Success
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: Index Exception", "[index_exception]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    
    nlohmann::json json = {{ url}, {7, 2, 5, 0}};
    library.add(json);

    try {
        json = { url, 10};
        library.get(json);
        FAIL("No exception was thrown");
    } catch (const IndexServerException& e) {
        // Success
    } catch (...) {
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: No array Exception", "[no_array_exception]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    
    nlohmann::json json = {{ url }, {1, "zs", {{"v","ret"}, {"hl",1}}}};
    library.add(json);


    try {
        json = { url, "v"};
        library.get(json);

        json = {{ url }, {}};
        library.add(json);

        FAIL("No exception was thrown");
    } catch (const IsNotArrayServerException& e) {
        json = {{ url }, {}};
        library.add(json);
        // Success
    } catch (...) {
        json = {{ url }, {}};
        library.add(json);
        FAIL("Unexpected exception was thrown");
    }
}

TEST_CASE("SERVER TEST: Delete data from galaaxy", "[delete_data_server]") {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;
    library.remove({});
    nlohmann::json addBody = nlohmann::json::array();
    nlohmann::json body = nlohmann::json::array();
    body.push_back(url);
    body.push_back("data");
    addBody.push_back(body);
    addBody.push_back({
        {"people", {
            {"name", "mike"},
            {"age", "10"}
        }},
        {"address", {
            {"city", "Bgd"},
            {"street", "Ledonr"}
        }}
    });
     cout << endl << addBody.dump() << endl;
    library.add(addBody);
    
    nlohmann::json deleteBody = { url, "data", "people", "name"};
    library.remove(deleteBody);

    nlohmann::json getBody = { url, "data"};
    auto result = library.get(getBody);

    nlohmann::json expectedResult = {
            {"people", {
                {"age", "10"}
            }},
            {"address", {
                {"city", "Bgd"},
                {"street", "Ledonr"}
            }}
    };

    REQUIRE(result == expectedResult);
    library.remove({ url });
}