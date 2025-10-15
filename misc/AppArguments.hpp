#pragma once

#include "Arguments.hpp"
#include "IniFile.hpp"
#include "str_replace.hpp"
#include "get_exec_path.hpp"

class AppArguments: public Arguments, public IniFile {
public:
    AppArguments(int argc, char* argv[]): 
        Arguments(argc, argv),
        IniFile(get_exec_path() + "/" + str_replace(
            {{ "{{PATH}}", get_exec_path() }}, 
            Arguments::get<string>("ini-file")
        ), true, false, true)
    {}

    virtual ~AppArguments() {}

    template<typename T, typename ...Args>
    T get(const string& key, Args... args) const {
        try {
            return Arguments::has(key) ? Arguments::get<T>(key, args...) : IniFile::get<T>(key);
        } catch (exception &e) {
            throw ERROR("Application argument " + EMPTY_OR(key) + " not found. The argument is missing. It must be specified in the " + EMPTY_OR(IniFile::getFilenameCRef()) + " file or provided as a command-line argument.");         
        }
    }

    template<typename T, typename ...Args>
    T get(int at, Args... args) const {
        return Arguments::get<T>(at, args...);
    }

    template<typename ...Args>
    bool has(const string& key, Args... args) const {
        return IniFile::has(key) || Arguments::has(key, args...);
    }

    template<typename ...Args>
    bool has(int at, Args... args) const {
        return Arguments::has(at, args...);
    }
};
