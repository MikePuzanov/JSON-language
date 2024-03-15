#include "json-language.h"
#include <nlohmann/json.hpp>
#include <iostream>

using namespace std;

int main() {
    //setlocale(LC_CTYPE, "rus");
    setlocale(LC_ALL, "Russian");
    //SetConsoleCP(866);
    //SetConsoleOutputCP(866);
    MyLibrary library;
    
    nlohmann::json json = 
    {
        {
            "http://127.0.0.1:4000"
        },
        {
            {
                {"one", {"two", 3}}
            }
        }
    };
    cout << "˜˜˜˜˜˜";
    //cout << "JSON content:\n" << json.dump(4) << endl;
    library.add(json);

    // json = { { "http://0.0.0.0:18080", "one" } };
    // library.get(json);

    // json = { { "http://0.0.0.0:18080" }, {{"one"}, {"two", 1412124214}}};
    // library.add(json);

    // json = { { "http://0.0.0.0:18080", "one" } };
    // library.get(json);

    // json = { {}, { {"one"}, {"two", 3} } };
    // library.add(json);

    // json = { {"one"} };
    // library.get(json);
}