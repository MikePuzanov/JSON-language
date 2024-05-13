#include <iostream>
#include <json.hpp>
#include "Teachers.h"

using namespace std;
using namespace nlohmann;

int main() {
    Teachers lib = Teachers("galaxy.json");
    json query = json::array();
    query.push_back("data");
    query.push_back("user1");
    json urls = nlohmann::json::array();
    urls.push_back("http://127.0.0.1:4000");
    json value = nlohmann::json::array();
    value.push_back("value");
    cout << endl << query.dump() << endl;
    cout << endl << value.dump() << endl;
    cout << endl << urls.dump() << endl;
    lib.addToServers(query, value, "http://127.0.0.1:4000");
    json queryGet = json::array();
    queryGet.push_back("data");
    queryGet.push_back("user1");
    cout << endl << queryGet.dump() << endl;
    json result = lib.get(queryGet);
    cout << endl << result.dump() << endl;
    json queryRemove = json::array();
    queryRemove.push_back("data");
    queryRemove.push_back("user1");
    cout << endl << "MAIN" << queryRemove.dump() << endl;
    lib.remove(queryRemove);
}