#pragma once

#include <string>
#include <iostream>
#include <filesystem> // Requires C++17 or later
#include <sys/stat.h> // For chmod (permissions)
#include "ERROR.hpp"
#include "str_cut_begin.hpp"

using namespace std;

namespace fs = filesystem; // Alias for convenience

bool remove(const string& path, bool throws = true) {
    bool ok = fs::remove(path);
    if (!ok && throws) throw ERROR("Unable to remove: " + str_cut_begin(path));
    return ok;
}
