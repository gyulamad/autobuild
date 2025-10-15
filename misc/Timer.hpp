#pragma once

#include "datetime_defs.hpp"
#include "Stopper.hpp"

class Timer {
public:
    template<typename T>
    void check(T cb, time_ms ms) {
        // If no callback provided, nothing to do
        // if (cb == nullptr) return;
        
        // If stopper is not running, start it for the first time
        if (!stopper.isRunning()) {
            stopper.start();
            return; // Don't call callback on first check
        }
        
        // Check if enough time has elapsed
        double elapsedMs = stopper.getElapsedMs();
        if (elapsedMs >= ms) {
            // Call the callback
            cb();
            
            // Reset the stopper to start timing for next interval
            stopper.reset();
            stopper.start();
        }
    }
private:
    Stopper stopper;
};
