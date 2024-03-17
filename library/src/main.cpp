#include "json-language.h"
#include <nlohmann/json.hpp>
#include <iostream>

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    MyLibrary library;

     nlohmann::json json = {{ "http://127.0.0.1:4000" }, {1, "zs", {{"v","ret"}, {"hl",1}}}};
    library.add(json);
    json = { "http://127.0.0.1:4000", "v"};
    library.get(json);
}
    // ���������� json
    // nlohmann::json json = 
    // {
    //     {
    //         "http://127.0.0.1:4000"
    //     },
    //     {
    //        {"one", {"two", 3}}
    //     }
    // };
    // library.add(json);
    // cout << "1 ��������" << endl;
    
    // // ���������� json
    // json = 
    // { 
    //     "http://127.0.0.1:4000", "one"
    // };
    // cout << endl << "2 �����" << endl;
    // nlohmann::json result = library.get(json);
    // cout << "2 ����� - " + result.dump() << endl;

    // // ����� 2 �������� � �������
    // json = 
    // { 
    //     { 
    //         "http://127.0.0.1:4000", "one", 2 
    //     },
    //     1412124214
    // };
    // cout << endl << "3 �����" << endl;
    // library.add(json);
    // cout << "3 �����" << endl;

    // // ��������� 2 �������� � �������
    // json = { "http://127.0.0.1:4000", "one" };
    // cout << endl << "4 �����" << endl;
    // result = library.get(json);
    // cout << "4 ����� - " + result.dump() << endl;

    // // ���������� ������ �������� � ������
    // json = 
    // { 
    //     {
    //        "http://127.0.0.1:4000", "one", 2 
    //     },  
    //     0
    // };
    // cout << endl << "5 �����" << endl;
    // library.add(json);
    // cout << "5 �����" << endl;


    // // ��������� json
    // json = { "http://127.0.0.1:4000", "one", 2 };
    // cout << endl << "6 �����" << endl;
    // result = library.get(json);
    // cout << "6 ����� - " + result.dump() << endl;



    // cout << endl << endl << "������ ��������";
    // // ���������� json
    // json = 
    // {
    //     {
        
    //     },
    //     {
    //        {"one", {"two", 3}}
    //     }
    // };
    // library.add(json);
    // cout << "1 ��������" << endl;
    
    // // ���������� json
    // json = 
    // { 
    //     "one"
    // };
    // cout << endl << "2 �����" << endl;
    // result = library.get(json);
    // cout << "2 ����� - " + result.dump() << endl;

    // // ����� 2 �������� � �������
    // json = 
    // { 
    //     { 
    //         "one", 2 
    //     },
    //     1412124214
    // };
    // cout << endl << "3 �����" << endl;
    // library.add(json);
    // cout << "3 �����" << endl;

    // // ��������� 2 �������� � �������
    // json = { "one" };
    // cout << endl << "4 �����" << endl;
    // result = library.get(json);
    // cout << "4 ����� - " + result.dump() << endl;

    // // ���������� ������ �������� � ������
    // json = 
    // { 
    //     {
    //        "one", 2 
    //     },  
    //     0
    // };
    // cout << endl << "5 �����" << endl;
    // library.add(json);
    // cout << "5 �����" << endl;


    // // ��������� json
    // json = { "one", 2 };
    // cout << endl << "6 �����" << endl;
    // result = library.get(json);
    // cout << "6 ����� - " + result.dump() << endl;