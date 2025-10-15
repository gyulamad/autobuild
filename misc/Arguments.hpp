#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <map>

#include "ERROR.hpp"
#include "str_starts_with.hpp"
#include "parse.hpp"
#include "safe.hpp"
#include "foreach.hpp"

using namespace std;

class Arguments {
public:
    using Key = pair<string, string>;
    struct Helps {
        vector<pair<string, string>> at;
        map<Key, string> key;
    };

    // Constructor that initializes from command-line arguments
    Arguments(
        int argc, char* argv[], 
        Helps* helps = nullptr, 
        const string& prefix = "--", 
        const string& prefix_short = "-",
        const string& equal_to = "="
    ):
        helps(helps),
        prefix(prefix),
        prefix_short(prefix_short),
        equal_to(equal_to)
    {
        for (int i = 0; i < argc; i++) 
            args.push_back(string(argv[i]));
    }

    virtual ~Arguments() {}

    void setHelps(Helps* helps) {
        this->helps = SAFE(helps);
    }

    void addHelp(const size_t& at, const string& name, const string& description) {
        SAFE(helps);
        if (helps->at.size() <= at) helps->at.resize(at + 1);
        helps->at[at] = { name, description };
    }

    void addHelp(const Key& key, const string& description) {
        SAFE(helps);
        helps->key[key] = description;
    }

    bool hasHelp(const string& key) const {
        SAFE(helps);
        for (const pair<Key, string> hlp: helps->key)
            if (hlp.first.first == key || hlp.first.second == key)
                return true;
        return false;
        // return helps->key.count({key, ""}) > 0 || std::any_of(helps->key.begin(), helps->key.end(), [&](const auto& pair){ return pair.first.first == key; });
    }

    bool hasHelp(const size_t at) const {
        SAFE(helps);
        return at <= helps->at.size();
    }

    const vector<string>& getArgsCRef() const {
        return args;
    }

    // Check if an argument exists
    bool has(const string& key, const string& prefix = "--") const {
        if (!hasHelp(key))
            throw ERROR("Help is not provoded for argument '" + prefix + key + "'");
        string prefixed_key = prefix + key;
        for (const auto& arg : args) 
            if (arg == prefixed_key || str_starts_with(arg, prefixed_key + equal_to)) 
                return true;
        return false;
    }

    bool has(const Key& keys) const {
        return has(keys.first, prefix) || has(keys.second, prefix_short);
    }

    // Check if an argument exists
    bool has(size_t at) const {
        if (!hasHelp(at))
            throw ERROR("Help is not provided for argument at " + to_string(at));
        return at < args.size();
    }

    // Get the index of a specific argument
    long int indexOf(const string& arg, const string& prefix = "--") const {
        string prefixed_key = prefix + arg;
        for (size_t i = 0; i < args.size(); ++i)
            if (args[i] == prefixed_key || str_starts_with(args[i], prefixed_key + equal_to)) 
                return (long int)(i);
        return -1;
    }
    
    long int indexOf(const Key& keys) const {
        long int longIdx = indexOf(keys.first);
        if (longIdx != -1) return longIdx;
        return indexOf(keys.second);
    }

    //  =================================== Bool (deprecated) ===================================

    // Get a boolean value based on the presence of a flag
    bool getBool(const string& key) const {
        return has(key);
    }

    // Get a boolean value based on the presence of a flag (with default value)
    bool getBool(const string& key, bool defval) const {
        return has(key) ? true : defval;
    }

    // Get a boolean value at position
    bool getBool(size_t at) const {
        if (has(at)) 
            throw ERROR("Missing argument at: " + to_string(at) + "\n" + help());
        return parse<bool>(args[at]);
    }

    // Get a boolean value at position (with default value)
    bool getBool(size_t at, bool defval) const {
        return has(at) ? defval : parse<bool>(args[at]);
    }

    //  =================================== Templated getters ===================================

    // Templated getter for key-based lookup
    template<typename T>
    T get(const string& key, const string& prefix = "--") const {
        long int idx = indexOf(key, prefix);
        if (idx == -1)
            throw ERROR("Missing argument: " + key + "\n" + help());

        string arg = args[idx];
        string prefixed_key = prefix + key + equal_to;
        if (str_starts_with(arg, prefixed_key)) {
            // Extract value after "="
            string value = arg.substr(prefixed_key.length());
            if (value.empty()) 
                throw ERROR("Missing value for argument: " + key + "\n" + help());
            return parse<T>(value);
        } else if (idx + 1 < (long int)(args.size()))
            // Value is in the next argument
            return parse<T>(args[idx + 1]);
        else
            throw ERROR("Missing value for argument: " + key + "\n" + help());
    }
    
    template<typename T>
    T get(const Key& keys) const {
        if (has(keys.first, prefix)) return get<T>(keys.first, prefix);
        else if (has(keys.second, prefix_short)) return get<T>(keys.second, prefix_short);
        else 
            throw ERROR("Missing argument: " + prefix + keys.first + " (or " + prefix_short + keys.second + ")" + "\n" + help());
    }

    // Templated getter with default value for key-based lookup
    template<typename T>
    T get(const string& key, const T& defval, const string& prefix = "--") const {
        return has(key, prefix) ? get<T>(key, prefix) : defval;
    }

    template<typename T>
    T get(const Key& keys, const T& defval) const {
        return has(keys) ? get<T>(keys) : defval;
    }

    // Templated getter for positional lookup
    template<typename T>
    T get(size_t at) const {
        if (!has(at)) 
            throw ERROR("Missing argument at: " + to_string(at) + "\n" + help());
        return parse<T>(args[at]);
    }

    // Templated getter with default value for positional lookup
    template<typename T>
    T get(size_t at, const T& defval) const {
        return !has(at) ? defval : parse<T>(args[at]);
    }

    string help() const {
        SAFE(helps);
        string output = "Usages:\n" + args[0];
        for (size_t i = 0; i < helps->at.size(); i++)
            output += " <" + helps->at[i].first + ">";
        if (!helps->key.empty()) output += " <options>\n";
        for (size_t i = 0; i < helps->at.size(); i++)
            output += helps->at[i].first + ": " + helps->at[i].second + "\n";
        if (!helps->key.empty()) output += "options:\n";
        for (const auto& [key, description] : helps->key) {
            output += prefix + key.first 
                + ", " + prefix_short + key.second + ": " 
                + description + "\n";
        }
        return output;
    }

protected:
    vector<string> args;
    Helps* helps = nullptr;
    string prefix = "--";
    string prefix_short = "-";
    string equal_to = "=";
};
    
template<>
inline bool Arguments::get<bool>(const string& key, const string& prefix) const {
    return has(key, prefix);
}

template<>
inline bool Arguments::get<bool>(const string& key, const bool& defval, const string& prefix) const {
    return has(key, prefix) ? true : defval;
}

template<>
inline bool Arguments::get<bool>(const Key& keys) const {
    return has(keys);
}

template<>
inline bool Arguments::get<bool>(const Key& keys, const bool& defval) const {
    return has(keys) ? true : defval;
}


#ifdef TEST

#include "str_contains.hpp"

TEST(test_Arguments_has_found) {
    vector<string> arg_strings = {"program", "--flag"};
    vector<char*> argv;
    for (string& str : arg_strings) {
        argv.push_back(const_cast<char*>(str.c_str()));
    }
    Arguments::Helps helps;
    Arguments args(static_cast<int>(argv.size()), argv.data(), &helps);
    args.addHelp({"flag", ""}, "Flag description");
    bool actual = args.has("flag");
    bool expected = true;
    assert(actual == expected && "Flag should be found");
}

TEST(test_Arguments_has_not_found) {
    vector<string> arg_strings = {"program", "--flag"};
    vector<char*> argv;
    for (string& str : arg_strings) {
        argv.push_back(const_cast<char*>(str.c_str()));
    }
    Arguments::Helps helps;
    Arguments args(static_cast<int>(argv.size()), argv.data(), &helps);
    args.addHelp({"flag", ""}, "Flag description");
    args.addHelp({"missing", ""}, "We expect but user is not providing this");
    bool actual = args.has("missing");
    bool expected = false;
    assert(actual == expected && "Missing flag should not be found");
}

TEST(test_Arguments_indexOf_found) {
    vector<string> arg_strings = {"program", "--flag"};
    vector<char*> argv;
    for (string& str : arg_strings) {
        argv.push_back(const_cast<char*>(str.c_str()));
    }
    Arguments::Helps helps;
    Arguments args(static_cast<int>(argv.size()), argv.data(), &helps);
    args.addHelp({"flag", ""}, "Flag description");
    long int actual = args.indexOf("flag");
    long int expected = 1;
    assert(actual == expected && "Index of flag should be 1");
}

TEST(test_Arguments_indexOf_not_found) {
    vector<string> arg_strings = {"program", "--flag"};
    vector<char*> argv;
    for (string& str : arg_strings) {
        argv.push_back(const_cast<char*>(str.c_str()));
    }
    Arguments::Helps helps;
    Arguments args(static_cast<int>(argv.size()), argv.data(), &helps);
    args.addHelp({"flag", ""}, "Flag description");
    long int actual = args.indexOf("missing");
    long int expected = -1;
    assert(actual == expected && "Index of missing flag should be -1");
}

// ----- Tests for short flags -----

// Helper to create Arguments from a vector of strings
Arguments createArgs(const vector<string>& input) {
    vector<char*> argv;
    for (auto& s : input) argv.push_back(const_cast<char*>(s.c_str()));
    return Arguments(static_cast<int>(argv.size()), argv.data());
}

// Test has() with short flag
TEST(test_Arguments_has_short_flag) {
    Arguments args = createArgs({"program", "-v"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    args.addHelp({ "verbose", "v" }, "Verbose description");
    bool actual = args.has(pair("verbose", "v"));
    assert(actual == true && "Short flag -v should be detected");
}

// Test has() with missing short flag
TEST(test_Arguments_has_short_flag_missing) {
    Arguments args = createArgs({"program"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    args.addHelp({ "verbose", "v" }, "Verbose description");
    bool actual = args.has(pair("verbose", "v"));
    assert(actual == false && "Missing short flag -v should return false");
}

// Test get<bool> with short flag
TEST(test_Arguments_get_bool_short_flag) {
    Arguments args = createArgs({"program", "-v"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    args.addHelp({ "verbose", "v" }, "Verbose description");
    bool actual = args.get<bool>(pair("verbose", "v"));
    assert(actual == true && "Short flag -v should return true for bool");
}

// Test get<bool> with short flag and default value
TEST(test_Arguments_get_bool_short_flag_default) {
    Arguments args = createArgs({"program"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    args.addHelp({ "verbose", "v" }, "Verbose description");
    bool actual = args.get<bool>(pair("verbose", "v"), false);
    assert(actual == false && "Missing short flag -v should return default false");
}

// Test get<int> with short flag and value
TEST(test_Arguments_get_int_short_flag_value) {
    Arguments args = createArgs({"program", "-c", "42"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    args.addHelp({ "count", "c" }, "Count description");
    int actual = args.get<int>(pair("count", "c"));
    assert(actual == 42 && "Short flag -c with value 42 should return 42");
}

// Test get<int> with short flag missing value
TEST(test_Arguments_get_int_short_flag_missing_value) {
    Arguments args = createArgs({"program", "-c"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    helps.key[{"count", "c"}] = "Test argument";
    bool thrown = false;
    string what;
    try {
        args.get<int>(pair("count", "c"));
    } catch (exception& e) {
        thrown = true;
        what = e.what();
        assert(str_contains(what, "Missing value for argument: c") && "Exception message should indicate missing value for -c");
    }
    assert(thrown && "Short flag -c without value should throw");
}

// Test get<string> with short flag and value
TEST(test_Arguments_get_string_short_flag_value) {
    Arguments args = createArgs({"program", "-f", "data.txt"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    args.addHelp({ "file", "f" }, "File description");
    string actual = args.get<string>(pair("file", "f"));
    assert(actual == "data.txt" && "Short flag -f with value data.txt should return data.txt");
}

// Test get<int> with short flag and default value
TEST(test_Arguments_get_int_short_flag_default) {
    Arguments args = createArgs({"program"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    args.addHelp({ "count", "c" }, "Count description");
    int actual = args.get<int>(pair("count", "c"), 10);
    assert(actual == 10 && "Missing short flag -c should return default 10");
}

// Test get<int> with both long and short flags present (long takes precedence)
TEST(test_Arguments_get_int_short_and_long_flags) {
    Arguments args = createArgs({"program", "--count", "100", "-c", "42"});
    Arguments::Helps helps;
    args.setHelps(&helps);
    args.addHelp({ "count", "c" }, "Count description");
    int actual = args.get<int>(pair("count", "c"));
    assert(actual == 100 && "Long flag --count should take precedence over -c");
}

#endif