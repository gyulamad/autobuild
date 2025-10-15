#pragma once

#include <string>
#include <chrono>
#include "filemtime_sec.hpp"
#include "datetime_defs.hpp"

using namespace std;

time_sec filemtime(const string& filename) {
    return filemtime_sec(filename);
}
