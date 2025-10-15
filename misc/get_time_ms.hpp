#pragma once

#include <thread>

#include "datetime_defs.hpp"
#include "default_get_time_ms.hpp"

using namespace std;

// Declaration in the header file (e.g., datetime.hpp)
extern time_ms (*get_time_ms)();

time_ms (*get_time_ms)() = default_get_time_ms;


#ifdef TEST

// Test that get_time_ms returns a reasonable current time
TEST(test_get_time_ms_current) {
    // Get current time using the function
    time_ms t1 = get_time_ms();
    
    // Get current time using standard chrono
    auto now = chrono::system_clock::now();
    auto millis = chrono::duration_cast<chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    
    // The two times should be very close
    // Allow small tolerance of 100ms for execution time difference
    time_ms diff = t1 > millis ? t1 - millis : millis - t1;
    assert(diff < 100 && "get_time_ms should return approximately current time");
}

// Test that consecutive calls to get_time_ms are monotonically increasing
TEST(test_get_time_ms_monotonic) {
    time_ms t1 = get_time_ms();
    this_thread::sleep_for(chrono::milliseconds(10));
    time_ms t2 = get_time_ms();
    
    assert(t2 > t1 && "get_time_ms should be monotonically increasing");
}

// Test time elapses correctly
TEST(test_get_time_ms_time_elapsed) {
    time_ms start = get_time_ms();
    this_thread::sleep_for(chrono::milliseconds(100));
    time_ms end = get_time_ms();
    
    time_ms elapsed = end - start;
    assert(elapsed >= 100 && "At least 100ms should have elapsed");
    // Allow some overhead for thread scheduling, but not too much
    assert(elapsed < 200 && "Elapsed time should be reasonable");
}

#endif
