#pragma once

#include "datetime_defs.hpp"
#include <chrono>

using namespace std;
using namespace chrono;

// Definition in the source file (e.g., datetime.cpp)
time_ms default_get_time_ms() {
    // Get the current time point
    auto now = chrono::system_clock::now();

    // Convert to milliseconds since the Unix epoch
    auto millis = chrono::duration_cast<chrono::milliseconds>(
        now.time_since_epoch()
    );

    // Return the numeric value
    return millis.count();
}
