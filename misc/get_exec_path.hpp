#pragma once

#include <string>
#include <limits.h>
#include <unistd.h>

#include "get_path.hpp"

using namespace std;

inline string get_exec_path() {
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
        return get_path(string(path));
    }
    return "";  
}