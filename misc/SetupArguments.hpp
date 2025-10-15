#pragma once

#include "Arguments.hpp"
#include "IniFile.hpp"

// Deprecated: use AppArguments instead
class SetupArguments: public Arguments {
public:
    SetupArguments(int argc, char* argv[]): Arguments(argc, argv) {
        if (has("setup")) {
            setupIniName = get<string>("setup");
            setupIniFile = IniFile(setupIniName, true);
        }
    }

    virtual ~SetupArguments() {}

    template<typename T, typename ...Args>
    T get(const string& key, Args... args) const {
        try {
            return setupIniFile.has(key) ? setupIniFile.get<T>(key) : Arguments::get<T>(key, args...);
        } catch (exception &e) {
            throw ERROR("Setup Argument " + EMPTY_OR(key) + " not found. The argument is missing. It must be specified in the " + EMPTY_OR(setupIniName, "<undefined setup ini file>") + " file or provided as a command-line argument.");         
        }
    }

    template<typename T, typename ...Args>
    T get(int at, Args... args) const {
        return Arguments::get<T>(at, args...);
    }

    template<typename ...Args>
    bool has(const string& key, Args... args) const {
        return setupIniFile.has(key) || Arguments::has(key, args...);
    }

    template<typename ...Args>
    bool has(int at, Args... args) const {
        return Arguments::has(at, args...);
    }

protected:
    string setupIniName;
    IniFile setupIniFile;
};
