#pragma once 

#include <functional>
#include <string>

using namespace std;

string get_hash(const string& str) {
    return to_string(hash<string>{}(str));
}
