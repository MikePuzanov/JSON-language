#include "JsonLanguage.h"
#include <nlohmann/json.hpp>
#include <iostream>

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    JsonLanguage library;

    nlohmann::json json = 
    {
        {
            "http://127.0.0.1:4000"
        },
        {
           {"one", {"two", 3}}
        }
    };
    library.add(json);
    cout << "1 command \"Add\". JSON = " + json.dump() << endl;
    

    json = 
    { 
        "http://127.0.0.1:4000", "one"
    };
    cout << endl << "2 command \"Get\". JSON = " + json.dump() << endl;
    nlohmann::json result = library.get(json);
    cout << "2 command. Result = " + result.dump() << endl;

    json = 
    { 
        { 
            "http://127.0.0.1:4000", "one", 2 
        },
        1412124214
    };
    cout << "3 command \"Add\". JSON = " + json.dump() << endl;
    library.add(json);


    json = { "http://127.0.0.1:4000", "one" };
    cout << "4 command \"Get\". JSON = " + json.dump() << endl;
    result = library.get(json);
    cout << "4 command. Result = " + result.dump() << endl;


    json = 
    { 
        {
           "http://127.0.0.1:4000", "one", 2 
        },  
        0
    };
    cout << "5 command \"Add\". JSON = " + json.dump() << endl;
    library.add(json);


    json = { "http://127.0.0.1:4000", "one", 2 };
    cout << "6 command \"Get\". JSON = " + json.dump() << endl;
    result = library.get(json);
    cout << "6 command. Result = " + result.dump() << endl;


    json = 
    {
        {
        
        },
        {
           {"one", {"two", 3}}
        }
    };
    library.add(json);
    cout << "1 command \"Add\". JSON = " + json.dump() << endl;
    

    json = 
    { 
        "one"
    };
    cout << "2 command \"Get\". JSON = " + json.dump() << endl;
    result = library.get(json);
    cout << "2 command. Result = " + result.dump() << endl;


    json = 
    { 
        { 
            "one", 2 
        },
        1412124214
    };
    cout << "3 command \"Add\". JSON = " + json.dump() << endl;
    library.add(json);


    json = { "one" };
    cout << "4 command \"Get\". JSON = " + json.dump() << endl;
    result = library.get(json);
    cout << "4 command. Result = " + result.dump() << endl;

    
    json = 
    { 
        {
           "one", 2 
        },  
        0
    };
    cout << "5 command \"Add\". JSON = " + json.dump() << endl;
    library.add(json);


    // ��������� json
    json = { "one", 2 };
    cout << "6 command \"Get\". JSON = " + json.dump() << endl;
    result = library.get(json);
    cout << "6 command. Result = " + result.dump() << endl;
}