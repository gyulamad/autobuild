#pragma once

#include "ERROR.hpp"
#include "Logger.hpp"

#include <iostream>
#include <fstream>

using namespace std;

class ClogLogger: public Logger {
public:
    // Constructor with option to redirect clog to a file and control buffering
    ClogLogger(const string& filename = "", bool unbuffered = false): original_buffer(clog.rdbuf()) {
        if (!filename.empty()) {
            file.open(filename, ios::app);
            if (!file.is_open()) {
                throw ERROR("Unable to open log file for clog: " + filename);
            }
            clog.rdbuf(file.rdbuf()); // Redirect clog to file
        }
        if (unbuffered) {
            clog.rdbuf()->pubsetbuf(nullptr, 0); // Disable buffering
        }
    }

    ~ClogLogger() {
        // Restore original clog buffer
        clog.rdbuf(original_buffer);
        if (file.is_open()) {
            file.close();
        }
    }

    void write(const string& output) override {
        clog << output << endl;
    }

private:
    ofstream file;
    streambuf* original_buffer;
};