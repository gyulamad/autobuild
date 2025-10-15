#pragma once

#include "ERROR.hpp"
#include "array_key_exists.hpp"
#include "array_keys.hpp"
#include "in_array.hpp"
#include "implode.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

using namespace std;

class Enumerator {
public:
    Enumerator(vector<string> values): values(values) {
        if (values.empty())
            throw ERROR("Enumerator values can not be empty");
    }
    virtual ~Enumerator() {}

    void select(const string& option, unordered_map<string, function<void()>> options, bool strict = true) const {
        if (!array_key_exists(option, options))
            throw ERROR("Enumerator selection does not cover the option: '" + option + "', " + available_options());
        if (strict && !in_array(option, values)) 
            throw ERROR("Enumerator option is invalid: '" + option + "', " + available_options());
        if (strict)
            for (const string& option_key: array_keys(options))
                if (!in_array(option_key, values))
                    throw ERROR("Invalid enumerator option requested: '" + option_key + "', " + available_options());
        options[option]();
    }

protected:
    vector<string> values;
    string available_options() const {
        return "available option(s) are: '" + implode("', '", values) + "'.";
    }
};
