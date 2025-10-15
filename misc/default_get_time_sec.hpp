#pragma once

#include <string>

#include "datetime_defs.hpp"
#include "get_time_ms.hpp"

using namespace std;

// Definition in the source file (e.g., datetime.cpp)
time_sec default_get_time_sec() {
    return get_time_ms() / second_ms;
}

