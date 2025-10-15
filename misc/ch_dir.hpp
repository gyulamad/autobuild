#pragma once

#include <string>
#include <cerrno>
#include <unistd.h>
#include <string.h>

#include "ERROR.hpp"

using namespace std;

void ch_dir(const string& dir) {
    if (chdir(dir.c_str()) == -1)
        throw ERROR("Failed to change directory: " + strerror(errno));
}