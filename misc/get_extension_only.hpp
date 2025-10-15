#pragma once

#include <string>
#include <vector>

#include "explode.hpp"
#include "fix_path.hpp"

using namespace std;

string get_extension_only(const string& filename) {
    if (filename.empty()) return "";
    vector<string> splits = explode("/", fix_path(filename));
    if (splits.empty()) return "";
    string last = splits[splits.size() - 1]; 
    splits = explode(".", last);
    if (splits.size() < 2) return "";
    return splits[splits.size() - 1];
}
