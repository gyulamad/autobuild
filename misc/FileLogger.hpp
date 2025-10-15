#pragma once

#include "ERROR.hpp"
#include "Logger.hpp"

#include <fstream>

using namespace std;

class FileLogger: public Logger {
public:
    FileLogger(const string& filename) : filename(filename) {
        file.open(filename, ios::app);
        if (!file.is_open())
            throw ERROR("Unable to open log file: " + filename);
    }

    ~FileLogger() {
        if (file.is_open()) file.close();
    }

    void write(const string& output) override {
        if (file.is_open()) {
            file << output << endl;
            file.flush();
        }
    }

private:
    ofstream file;
    string filename;
};