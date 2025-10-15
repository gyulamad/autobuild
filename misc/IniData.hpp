#pragma once

#include "array_keys.hpp"
#include "array_values.hpp"
#include "array_key_exists.hpp"
#include "parse.hpp"

#include <string>

using namespace std;

class IniData {
public:
    IniData() {}
    virtual ~IniData() {}

    vector<string> sections() const {
        return array_keys(data);
    }

    void setData(const IniData& other) {
        changed = data != other.data;
        data = other.data;
        // TODO: changed?
    }

    void clear() {
        if (!data.empty()) changed = true;
        data.clear();
    }

    size_t size(const string& section = "") const {
        return data.at(section).size();
    }

    bool has(const string& key, const string& section = "") const {
        return in_array(section, sections()) && array_key_exists(key, data.at(section));
    }

    vector<string> keys(const string& section = "") const {
        return array_keys(data.at(section));
    }

    vector<string> values(const string& section = "") const {
        if (!array_key_exists(section, data))
            throw ERROR("Section is not exists: " + EMPTY_OR(section, "<global>"));
        return array_values(data.at(section));
    }

    template<typename T>
    unordered_map<string, T> list(const string& section = "") const {
        unordered_map<string, T> typed;
        for (const auto& [key, value]: data.at(section))
            typed[key] = parse<T>(value);
        return typed;
    }

    template<typename T>
    T get(const string& key, const string& section = "") const {
        if (!has(key, section)) 
            throw ERROR("Key is not exists: " + EMPTY_OR(key) + " in section [" + EMPTY_OR(section, "<global>") + "]");
        return parse<T>(data.at(section).at(key));
    }

    template<typename T>
    void set(const string& key, T value, const string& section = "") {
        string newval = this->as_string(value);
        if (newval != data[section][key]) {
            data[section][key] = newval;
            changed = true;
        }
    }

protected:

    unordered_map<
        string, 
        unordered_map<string, string>
    > data;
    bool changed = false;
    
private:
    inline string as_string(const string& str) { return str; }
    inline string as_string(const char* str) { return str; }

    template<typename T>
    inline string as_string(T str) { return std::to_string(str); }
};
