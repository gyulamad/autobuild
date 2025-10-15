#pragma once

#include <thread>
#include <chrono>
#include <stdexcept>

#include "ERROR.hpp"

using namespace std;

class Stopper {
public:
    Stopper(bool start = true) { if (start) this->start(); }
    virtual ~Stopper() {}

    // Start the stopwatch (throws if already running)
    void start() {
        if (started)
            throw ERROR("Stopper start error, stopper is already started.");
        started = true;
        startTime = chrono::high_resolution_clock::now();
    }

    // Pause the stopwatch (throws if not running)
    void pause() {
        if (!started)
            throw ERROR("Stopper pause error, stopper is not started.");
        paused = true;
        elapsed += chrono::duration_cast<chrono::nanoseconds>(
            chrono::high_resolution_clock::now() - startTime
        ).count();
    }

    // Resume the stopwatch (throws if already running)
    void resume() {
        if (!paused)
            throw ERROR("Stopper resume error, stopper is not paused");
        paused = false;
        startTime = chrono::high_resolution_clock::now();
    }

    // Stop the stopwatch and return elapsed time in milliseconds (throws if not running)
    double stop() {
        if (!started)
            throw ERROR("Stopper stop error, stopper is not started");
        started = false;
        elapsed += chrono::duration_cast<chrono::nanoseconds>(
            chrono::high_resolution_clock::now() - startTime
        ).count();
        double elapsedMs = elapsed / 1'000'000.0; // Convert nanoseconds to milliseconds
        // reset(); // Reset for next use
        return elapsedMs;
    }

    // Get elapsed time so far in milliseconds (works whether running or paused)
    double getElapsedMs() const {
        //if (started && !paused) {
        if (isRunning()) {
            auto current = elapsed + 
                chrono::duration_cast<chrono::nanoseconds>(
                    chrono::high_resolution_clock::now() - startTime
                ).count();
            return current / 1'000'000.0; // Convert to milliseconds
        }
        return elapsed / 1'000'000.0; // Convert to milliseconds
    }

    // Reset the stopwatch to initial state
    void reset() {
        started = true;
        paused = false;
        elapsed = 0;
        startTime = chrono::high_resolution_clock::now();
    }

    // Check if the stopwatch is running
    bool isRunning() const {
        return started && !paused;
    }

    string toString() const {
        double elapsedMs = getElapsedMs();
        // Round to nearest millisecond
        long long total_milliseconds = static_cast<long long>(elapsedMs + 0.5);

        if (total_milliseconds == 0) {
            return "0 milliseconds";
        }

        long long seconds_total = total_milliseconds / 1000;
        long long milliseconds = total_milliseconds % 1000;

        long long minutes_total = seconds_total / 60;
        long long seconds = seconds_total % 60;

        long long hours_total = minutes_total / 60;
        long long minutes = minutes_total % 60;

        long long days_total = hours_total / 24;
        long long hours = hours_total % 24;

        long long years = days_total / 365;
        long long days_remaining = days_total % 365;

        long long months = days_remaining / 30;
        days_remaining %= 30;

        long long weeks = days_remaining / 7;
        long long days = days_remaining % 7;

        vector<pair<long long, string>> parts;

        if (years > 0) 
            parts.push_back({years, years == 1 ? "year" : "years"});
        if (months > 0) 
            parts.push_back({months, months == 1 ? "month" : "months"});
        if (weeks > 0) 
            parts.push_back({weeks, weeks == 1 ? "week" : "weeks"});
        if (days > 0) 
            parts.push_back({days, days == 1 ? "day" : "days"});
        if (hours > 0) 
            parts.push_back({hours, hours == 1 ? "hour" : "hours"});
        if (minutes > 0) 
            parts.push_back({minutes, minutes == 1 ? "minute" : "minutes"});
        if (seconds > 0) 
            parts.push_back({seconds, seconds == 1 ? "second" : "seconds"});
        if (milliseconds > 0) 
            parts.push_back({milliseconds, milliseconds == 1 ? "millisecond" : "milliseconds"});

        string result;
        for (size_t i = 0; i < parts.size(); i++) {
            if (i > 0) {
                result += ", ";
            }
            result += to_string(parts[i].first) + " " + parts[i].second;
        }
        return result;
    }

#ifdef TEST
    void setElapsedMsForTesting(double ms) {
        if (started) {
            throw ERROR("Cannot set elapsed for testing while running");
        }
        elapsed = static_cast<long long>(ms * 1'000'000.0); // ms to nanoseconds
    }
#endif

private:
    bool started = false, paused = false;
    long long elapsed = 0; // Accumulated time in nanoseconds
    chrono::time_point<chrono::high_resolution_clock> startTime;
};

#ifdef TEST

#include "str_contains.hpp"

TEST(test_Stopper_start_stop_basic) {
    Stopper stopper(false);
    stopper.start();
    this_thread::sleep_for(chrono::milliseconds(100));
    double elapsed = stopper.stop();
    assert(elapsed >= 90 && elapsed <= 150 && "Elapsed time should be approximately 100ms");
    assert(!stopper.isRunning() && "Stopper should not be running after stop");
}

TEST(test_Stopper_pause_resume) {
    Stopper stopper(false);
    stopper.start();
    this_thread::sleep_for(chrono::milliseconds(50));
    stopper.pause();
    double elapsedPaused = stopper.getElapsedMs();
    assert(elapsedPaused >= 45 && elapsedPaused <= 75 && "Paused elapsed time should be approximately 50ms");
    stopper.resume();
    this_thread::sleep_for(chrono::milliseconds(50));
    double elapsed = stopper.stop();
    assert(elapsed >= 90 && elapsed <= 150 && "Total elapsed time should be approximately 100ms");
}

TEST(test_Stopper_getElapsedMs_running) {
    Stopper stopper(false);
    stopper.start();
    this_thread::sleep_for(chrono::milliseconds(100));
    double elapsed = stopper.getElapsedMs();
    assert(elapsed >= 90 && elapsed <= 150 && "Running elapsed time should be approximately 100ms");
    stopper.stop();
}

TEST(test_Stopper_getElapsedMs_paused) {
    Stopper stopper(false);
    stopper.start();
    this_thread::sleep_for(chrono::milliseconds(50));
    stopper.pause();
    double elapsed = stopper.getElapsedMs();
    assert(elapsed >= 45 && elapsed <= 75 && "Paused elapsed time should be approximately 50ms");
    stopper.resume();
    stopper.stop();
}

TEST(test_Stopper_reset) {
    Stopper stopper(false);
    stopper.start();
    this_thread::sleep_for(chrono::milliseconds(50));
    stopper.pause();
    stopper.reset();
    double elapsed = stopper.getElapsedMs();
    assert(elapsed < 0.01 && "Elapsed time should be almost 0 after reset");
    assert(stopper.isRunning() && "Stopper should be running after reset");
}

TEST(test_Stopper_start_already_running) {
    Stopper stopper(false);
    stopper.start();
    bool thrown = false;
    try {
        stopper.start();
    } catch (exception &e) {
        thrown = true;
        string what = e.what();
        assert(str_contains(what, "stopper is already started") && "Exception message should indicate already running");
    }
    assert(thrown && "Start while running should throw");
    stopper.stop();
}

TEST(test_Stopper_pause_not_running) {
    Stopper stopper(false);
    bool thrown = false;
    try {
        stopper.pause();
    } catch (exception &e) {
        thrown = true;
        string what = e.what();
        assert(str_contains(what, "stopper is not started") && "Exception message should indicate not running");
    }
    assert(thrown && "Pause while not running should throw");
}

TEST(test_Stopper_stop_not_running) {
    Stopper stopper(false);
    bool thrown = false;
    try {
        stopper.stop();
    } catch (exception &e) {
        thrown = true;
        string what = e.what();
        assert(str_contains(what, "stopper is not started") && "Exception message should indicate not running");
    }
    assert(thrown && "Stop while not running should throw");
}

TEST(test_Stopper_resume_already_running) {
    Stopper stopper(false);
    stopper.start();
    bool thrown = false;
    try {
        stopper.resume();
    } catch (exception &e) {
        thrown = true;
        string what = e.what();
        assert(str_contains(what, "stopper is not paused") && "Exception message should indicate can not resume");
    }
    assert(thrown && "Resume while running should throw");
    stopper.stop();
}

TEST(test_Stopper_immediate_stop) {
    Stopper stopper(false);
    stopper.start();
    double elapsed = stopper.stop();
    assert(elapsed >= 0 && elapsed <= 10 && "Immediate stop should have near-zero elapsed time");
    assert(!stopper.isRunning() && "Stopper should not be running after stop");
}

TEST(test_Stopper_toString_zero) {
    Stopper s(false);
    s.setElapsedMsForTesting(0.0);
    string result = s.toString();
    assert(result == "0 milliseconds");
}

TEST(test_Stopper_toString_1ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(1.0);
    string result = s.toString();
    assert(result == "1 millisecond");
}

TEST(test_Stopper_toString_2ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(2.0);
    string result = s.toString();
    assert(result == "2 milliseconds");
}

TEST(test_Stopper_toString_1000ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(1000.0);
    string result = s.toString();
    assert(result == "1 second");
}

TEST(test_Stopper_toString_1001ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(1001.0);
    string result = s.toString();
    assert(result == "1 second, 1 millisecond");
}

TEST(test_Stopper_toString_60000ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(60000.0);
    string result = s.toString();
    assert(result == "1 minute");
}

TEST(test_Stopper_toString_3600000ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(3600000.0);
    string result = s.toString();
    assert(result == "1 hour");
}

TEST(test_Stopper_toString_86400000ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(86400000.0);
    string result = s.toString();
    assert(result == "1 day");
}

TEST(test_Stopper_toString_604800000ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(604800000.0);
    string result = s.toString();
    assert(result == "1 week");
}

TEST(test_Stopper_toString_2592000000ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(2592000000.0);
    string result = s.toString();
    assert(result == "1 month");
}

TEST(test_Stopper_toString_31536000000ms) {
    Stopper s(false);
    s.setElapsedMsForTesting(31536000000.0);
    string result = s.toString();
    assert(result == "1 year");
}

TEST(test_Stopper_toString_combination) {
    Stopper s(false);
    // 1 year, 1 month, 1 week, 1 day, 1 hour, 1 minute, 1 second, 1 millisecond
    s.setElapsedMsForTesting(31536000000.0 + 2592000000.0 + 604800000.0 + 86400000.0 + 3600000.0 + 60000.0 + 1000.0 + 1.0);
    string result = s.toString();
    assert(result == "1 year, 1 month, 1 week, 1 day, 1 hour, 1 minute, 1 second, 1 millisecond");
}

TEST(test_Stopper_toString_rounding) {
    Stopper s(false);
    s.setElapsedMsForTesting(1.4);
    assert(s.toString() == "1 millisecond");
    s.setElapsedMsForTesting(1.5);
    assert(s.toString() == "2 milliseconds");
}

#endif
