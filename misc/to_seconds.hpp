#pragma once

#include <string>
#include "to_milliseconds.hpp"

using namespace std;

double to_seconds(const string& period_str) {
    return to_milliseconds(period_str) / 1000.0;
}

#ifdef TEST

TEST(test_to_seconds_simple) {
    double seconds = to_seconds("1 second");
    assert(seconds == 1.0);
}

#endif