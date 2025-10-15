#pragma once

#include <string>

using namespace std;

bool str_contains(const string& str, const string& substring) {
    // Use string::find to check if the substring exists
    return str.find(substring) != string::npos;
}
