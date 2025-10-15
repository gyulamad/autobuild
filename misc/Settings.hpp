#pragma once

#include "get_hash.hpp"
#include "JSON.hpp"
#include "JSONExts.hpp"
#include "Arguments.hpp"

#include <string>
#include <vector>
#include <map>
#include <type_traits>

using namespace std;

class Settings {
public:
    Settings() {}
    Settings(JSON& conf) : conf(&conf) {}
    Settings(Arguments& args) : args(&args) {}
    Settings(JSON& conf, Arguments& args) : args(&args), conf(&conf) {}
    Settings(Arguments& args, JSON& conf) : args(&args), conf(&conf) {}

    // Add a JSON extension
    void extends(JSON ext) {
        exts.extends(ext);
    }

    // Compute a hash of the settings
    string hash() const {
        return get_hash(
            (!exts.empty() ? exts.dump() : "<noexts>") +
            (conf ? conf->dump() : "<noconf>") +
            (args ? implode(" ", args->getArgsCRef()) : "<noargs>")
        );
    }

    // Check if a key exists
    bool has(const string& key) {
        if (args && args->has(key)) return true;
        if (!exts.empty() && exts.has(key)) return true;
        if (conf && conf->has(key)) return true;
        return false;
    }

    bool has(const pair<string, string>& keys) {
        if (args && args->has(keys)) return true;
        if (!exts.empty() && exts.has(keys.first)) return true;
        if (conf && conf->has(keys.first)) return true;
        return false;
    }

    template<typename T>
    T get(const string& key) {
        if constexpr (!is_container<T>::value) if (args && args->has(key)) return args->get<T>(key);
        if (!exts.empty() && exts.has(key)) return exts.get<T>(key);
        if (conf && conf->has(key)) return conf->get<T>(key);
        throw ERROR("Settings is missing for '" + key + "'");
    }

    template<typename T>
    T get(const string& key, const T defval) {
        if constexpr (!is_container<T>::value) if (args && args->has(key)) return args->get<T>(key);
        if (!exts.empty() && exts.has(key)) return exts.get<T>(key);
        if (conf && conf->has(key)) return conf->get<T>(key);
        return defval;
    }

    template<typename T>
    T get(const pair<string, string>& keys) {
        if constexpr (!is_container<T>::value) if (args && args->has(keys)) return args->get<T>(keys);
        if (!exts.empty() && exts.has(keys.first)) return exts.get<T>(keys.first);
        if (conf && conf->has(keys.first)) return conf->get<T>(keys.first);
        throw ERROR("Settings is missing for '" + keys.first + "' (or '" + keys.second + "')");
    }

    template<typename T>
    T get(const pair<string, string>& keys, const T defval) {
        if constexpr (!is_container<T>::value) if (args && args->has(keys)) return args->get<T>(keys);
        if (!exts.empty() && exts.has(keys.first)) return exts.get<T>(keys.first);
        if (conf && conf->has(keys.first)) return conf->get<T>(keys.first);
        return defval;
    }

    Arguments* args = nullptr;
    JSON* conf = nullptr;
    JSONExts exts;

private:
    // Default case: assume types are not containers
    template<typename T>
    struct is_container : false_type {};

    // Specialization for vector
    template<typename T>
    struct is_container<vector<T>> : true_type {};

    // Specialization for map
    template<typename K, typename V>
    struct is_container<map<K, V>> : true_type {};
};

#ifdef TEST

TEST(test_Settings_constructor_default) {
    Settings settings;
    assert(settings.args == nullptr && "Default constructor should set args to nullptr");
    assert(settings.conf == nullptr && "Default constructor should set conf to nullptr");
}

TEST(test_Settings_constructor_json) {
    JSON json;
    Settings settings(json);
    assert(settings.args == nullptr && "JSON constructor should set args to nullptr");
    assert(settings.conf == &json && "JSON constructor should set conf correctly");
}

TEST(test_Settings_constructor_args) {
    char* argv[] = {(char*)"program"};
    Arguments args(1, argv);
    Settings settings(args);
    assert(settings.args == &args && "Args constructor should set args correctly");
    assert(settings.conf == nullptr && "Args constructor should set conf to nullptr");
}

TEST(test_Settings_hash_with_both) {
    JSON json;
    json.set("key", "value");
    char* argv[] = {(char*)"program", (char*)"--test", (char*)"123"};
    Arguments args(3, argv);
    Settings settings(json, args);
    
    string conf_str = json.dump();
    string args_str = implode(" ", args.getArgsCRef());
    string actual_input = conf_str + args_str;
    string actual = settings.hash();
    
    string expected_input = "{\"key\":\"value\"}program --test 123";
    string expected = get_hash("<noexts>" + expected_input);
    
    assert(actual_input == expected_input && "Input string to hash function does not match expected");
    assert(actual == expected && "Hash should combine conf and args correctly");
}

TEST(test_Settings_hash_no_conf) {
    char* argv[] = {(char*)"program", (char*)"--test", (char*)"456"};
    Arguments args(3, argv);
    Settings settings(args);
    string actual = settings.hash();
    string expected = get_hash("<noexts><noconf>program --test 456");
    assert(actual == expected && "Hash should handle missing conf correctly");
}

TEST(test_Settings_has_single_key_args) {
    char* argv[] = {(char*)"program", (char*)"--flag"};
    Arguments args(2, argv);
    Settings settings(args);
    bool actual = settings.has("flag");
    assert(actual == true && "Should find key in args");
}

TEST(test_Settings_has_single_key_conf) {
    JSON json;
    json.set("key", "value");
    Settings settings(json);
    bool actual = settings.has("key");
    assert(actual == true && "Should find key in conf");
}

TEST(test_Settings_get_missing_key) {
    Settings settings;
    bool thrown = false;
    string what;
    
    try {
        settings.get<int>("missing");
    } catch (exception& e) {
        thrown = true;
        what = e.what();
        assert(str_contains(what, "Settings is missing for 'missing'") && "Exception message should match");
    }
    
    assert(thrown && "Get should throw when key is missing");
}

TEST(test_Settings_get_with_default) {
    Settings settings;
    int actual = settings.get<int>("missing", 42);
    assert(actual == 42 && "Should return default value when key is missing");
}

TEST(test_Settings_get_priority_args_over_conf) {
    JSON json;
    json.set("key", 1);
    char* argv[] = {(char*)"program", (char*)"--key", (char*)"2"};
    Arguments args(3, argv);
    Settings settings(json, args);
    int actual = settings.get<int>("key");
    assert(actual == 2 && "Args should take priority over conf");
}

TEST(test_Settings_get_pair_from_args) {
    char* argv[] = {(char*)"program", (char*)"--flag", (char*)"true"};
    Arguments args(3, argv);
    Settings settings(args);
    bool actual = settings.get<bool>(pair<string, string>("flag", "other"));
    assert(actual == true && "Should get value from args using pair key");
}

TEST(test_Settings_get_pair_missing) {
    Settings settings;
    bool thrown = false;
    string what;
    
    try {
        settings.get<int>(pair<string, string>("key1", "key2"));
    } catch (exception& e) {
        thrown = true;
        what = e.what();
        assert(str_contains(what, "Settings is missing for 'key1' (or 'key2')") && "Exception message should match");
    }
    
    assert(thrown && "Get with pair should throw when both keys are missing");
}

TEST(test_Settings_get_pair_with_default) {
    Settings settings;
    string actual = settings.get<string>(pair<string, string>("key1", "key2"), "default");
    assert(actual == "default" && "Should return default value when pair keys are missing");
}

TEST(test_Settings_constructor_args_conf) {
    JSON json;
    char* argv[] = {(char*)"program"};
    Arguments args(1, argv);
    Settings settings(args, json);
    assert(settings.args == &args && "Constructor should set args correctly");
    assert(settings.conf == &json && "Constructor should set conf correctly");
}

TEST(test_Settings_extends_adds_json) {
    Settings settings;
    JSON json;
    json.set("key", "value");
    settings.extends(json);
    bool actual = settings.has("key");
    assert(actual == true && "Extends should add JSON with key to exts");
}

TEST(test_Settings_has_returns_false) {
    Settings settings;
    bool actual = settings.has("missing");
    assert(actual == false && "Has should return false when key is missing in args, conf, and exts");
}

#endif
