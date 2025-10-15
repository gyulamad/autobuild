#pragma once

#include <string>
#include <chrono>
#include "filemtime_duration.hpp"
#include "datetime_defs.hpp"

using namespace std;

time_ms filemtime_ms(const string& filename) {
    return chrono::duration_cast<chrono::milliseconds>(filemtime_duration(filename)).count();
}
