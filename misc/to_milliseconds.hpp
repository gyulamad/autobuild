#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "ERROR.hpp"
#include "trim.hpp"

using namespace std;

static const unordered_map<string, double> to_milliseconds_unit_map = {
    {"year", 365 * 24 * 3600 * 1000.0},
    {"years", 365 * 24 * 3600 * 1000.0},
    {"y", 365 * 24 * 3600 * 1000.0},
    {"week", 7 * 24 * 3600 * 1000.0},
    {"weeks", 7 * 24 * 3600 * 1000.0},
    {"w", 7 * 24 * 3600 * 1000.0},
    {"day", 24 * 3600 * 1000.0},
    {"days", 24 * 3600 * 1000.0},
    {"d", 24 * 3600 * 1000.0},
    {"hour", 3600 * 1000.0},
    {"hours", 3600 * 1000.0},
    {"h", 3600 * 1000.0},
    {"minute", 60 * 1000.0},
    {"minutes", 60 * 1000.0},
    {"min", 60 * 1000.0},
    {"mins", 60 * 1000.0},
    {"m", 60 * 1000.0},
    {"second", 1000.0},
    {"seconds", 1000.0},
    {"s", 1000.0},
    {"millisecond", 1.0},
    {"milliseconds", 1.0},
    {"millis", 1.0},
    {"ms", 1.0}
};

vector<string> to_milliseconds_tokenize_period_string(const string& input) {
    vector<string> tokens;
    string buffer;
    
    for (char c : input) {
        if (isspace(c)) {
            if (!buffer.empty()) {
                tokens.push_back(buffer);
                buffer.clear();
            }
        }
        // Split when switching between digit/non-digit
        else if (!buffer.empty() && 
                (isdigit(c) != isdigit(static_cast<unsigned char>(buffer.back())))) {
            tokens.push_back(buffer);
            buffer = c;
        }
        else {
            buffer += c;
        }
    }
    
    if (!buffer.empty()) {
        tokens.push_back(buffer);
    }
    
    return tokens;
}

double to_milliseconds(const string& period_string) {
    string period_str = trim(period_string);
    if (period_str.empty()) return 0.0;

    vector<string> tokens = to_milliseconds_tokenize_period_string(period_str);
    if (tokens.size() % 2 != 0)
        throw ERROR("Input must have pairs of numbers and units");

    double total_ms = 0.0;
    for (size_t i = 0; i < tokens.size(); i += 2) {
        double value;
        try {
            value = stod(tokens[i]);
        } catch (...) {
            throw ERROR("Invalid number: " + tokens[i]);
        }

        string unit = tokens[i+1];
        transform(unit.begin(), unit.end(), unit.begin(), 
                  [](unsigned char c){ return tolower(c); });

        auto it = to_milliseconds_unit_map.find(unit);
        if (it == to_milliseconds_unit_map.end())
            throw ERROR("Invalid unit: " + unit);

        total_ms += value * it->second;
    }
    return total_ms;
}

#ifdef TEST

#include "str_contains.hpp"

TEST(test_to_milliseconds_empty_string) {
    assert(to_milliseconds("") == 0.0);
}

TEST(test_to_milliseconds_single_component_week) {
    double ms = to_milliseconds("1 week");
    assert(ms == 7 * 24 * 3600 * 1000.0);
}

TEST(test_to_milliseconds_multiple_components) {
    double ms = to_milliseconds("1 week 3 days");
    assert(ms == (7 + 3) * 24 * 3600 * 1000.0);
}

TEST(test_to_milliseconds_abbreviations) {
    double ms = to_milliseconds("1Y 2w 3d 4h 5m 6s 7ms");
    double expected = (365 * 24 * 3600 * 1000.0) 
                    + (2 * 7 * 24 * 3600 * 1000.0)
                    + (3 * 24 * 3600 * 1000.0)
                    + (4 * 3600 * 1000.0)
                    + (5 * 60 * 1000.0)
                    + (6 * 1000.0)
                    + 7.0;
    assert(ms == expected);
}

TEST(test_to_milliseconds_case_insensitivity) {
    double ms = to_milliseconds("1 WEEK");
    assert(ms == 7 * 24 * 3600 * 1000.0);
}

TEST(test_to_milliseconds_invalid_number_exception) {
    bool threw = false;
    try {
        to_milliseconds("abc day");
    } catch (const exception& e) {
        assert(str_contains(e.what(), "Invalid number: abc"));
        threw = true;
    }
    assert(threw);
}

TEST(test_to_milliseconds_invalid_unit_exception) {
    bool threw = false;
    try {
        to_milliseconds("1 abc");
    } catch (const exception& e) {
        assert(str_contains(e.what(), "Invalid unit: abc"));
        threw = true;
    }
    assert(threw);
}

TEST(test_to_milliseconds_odd_tokens_exception) {
    bool threw = false;
    try {
        to_milliseconds("1 day 2");
    } catch (const exception& e) {
        assert(str_contains(e.what(), "Input must have pairs of numbers and units"));
        threw = true;
    }
    assert(threw);
}

#endif
