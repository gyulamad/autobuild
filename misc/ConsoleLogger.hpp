#pragma once

#include "Logger.hpp"
#include <iostream>

using namespace std;

class ConsoleLogger: public Logger {
public:
    ConsoleLogger(): Logger() {}
    virtual ~ConsoleLogger() {}
protected:
    void write(const string& output) override {
        cout << output << endl;
    }

    string time() override { return ""; }
};