#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <limits>

#include "ERROR.hpp"
#include "str_cut_end.hpp"
#include "in_array.hpp"

using namespace std;

template <typename T>
T parse(const string& str) {
    if constexpr (is_same_v<T, string>) {
        return str; // Return the string directly
    } else {
        static_assert(is_arithmetic<T>::value, "T must be an arithmetic type");
        stringstream ss(str);
        T num;
        
        // Handle special floating-point values for float and double
        if constexpr (is_floating_point_v<T>) {
            string lower = str;
            transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower == "inf" || lower == "+inf") return numeric_limits<T>::infinity();
            if (lower == "-inf") return -numeric_limits<T>::infinity();
            if (lower == "nan") return numeric_limits<T>::quiet_NaN();
        }
        
        if (ss >> num)
            return num;
        
        throw ERROR("Invalid input string (not a number): " + EMPTY_OR(str));
    }
}

// Specialization for bool
template <>
bool parse<bool>(const string& str) {
    string lower = str;
    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);        
    if (in_array(lower, vector<string>({ "true", "on", "1", "yes"}))) return true;
    if (in_array(lower, vector<string>({ "false", "off", "0", "no"}))) return false;
    throw ERROR("Invalid input string (not a boolean): " + EMPTY_OR(str));
}

#ifdef TEST

#include <cmath>

TEST(test_parse_valid_integer) {
    int expected = 42;
    int actual = parse<int>("42");
    assert(actual == expected && "test_parse_valid_integer failed");
}

TEST(test_parse_valid_double) {
    double expected = 3.14;
    double actual = parse<double>("3.14");
    assert(actual == expected && "test_parse_valid_double failed");
}

TEST(test_parse_negative_number) {
    int expected = -10;
    int actual = parse<int>("-10");
    assert(actual == expected && "test_parse_negative_number failed");
}

TEST(test_parse_invalid_input) {
    bool thrown = false;
    try {
        parse<int>("abc");
    } catch (const exception&) {
        thrown = true;
    }
    assert(thrown && "test_parse_invalid_input failed");
}

TEST(test_parse_empty_string) {
    bool thrown = false;
    try {
        parse<double>("");
    } catch (const exception&) {
        thrown = true;
    }
    assert(thrown && "test_parse_empty_string failed");
}

TEST(test_parse_whitespace_only) {
    bool thrown = false;
    try {
        parse<int>("   ");
    } catch (const exception&) {
        thrown = true;
    }
    assert(thrown && "test_parse_whitespace_only failed");
}

TEST(test_parse_trailing_characters) {    
    int actual = parse<int>("42abc");
    assert(actual == 42 && "test_parse_trailing_characters failed");
}

TEST(test_parse_trailing_characters_negative) {    
    int actual = parse<int>("-42abc");
    assert(actual == -42 && "test_parse_trailing_characters_negative failed");
}

TEST(test_parse_floating_point_with_exponent) {
    double expected = 1.23e3;
    double actual = parse<double>("1.23e3");
    assert(actual == expected && "test_parse_floating_point_with_exponent failed");
}

TEST(test_parse_zero) {
    int expected = 0;
    int actual = parse<int>("0");
    assert(actual == expected && "test_parse_zero failed");
}

TEST(test_parse_large_number) {
    long long expected = 9223372036854775807LL;
    long long actual = parse<long long>("9223372036854775807");
    assert(actual == expected && "test_parse_large_number failed");
}

TEST(test_parse_positive_infinity_double) {
    double actual = parse<double>("inf");
    assert(isinf(actual) && actual > 0 && "test_parse_positive_infinity_double failed");
}

TEST(test_parse_positive_infinity_explicit_double) {
    double actual = parse<double>("+inf");
    assert(isinf(actual) && actual > 0 && "test_parse_positive_infinity_explicit_double failed");
}

TEST(test_parse_negative_infinity_double) {
    double actual = parse<double>("-inf");
    assert(isinf(actual) && actual < 0 && "test_parse_negative_infinity_double failed");
}

TEST(test_parse_nan_double) {
    double actual = parse<double>("nan");
    assert(isnan(actual) && "test_parse_nan_double failed");
}

TEST(test_parse_positive_infinity_float) {
    float actual = parse<float>("inf");
    assert(isinf(actual) && actual > 0 && "test_parse_positive_infinity_float failed");
}

TEST(test_parse_positive_infinity_explicit_float) {
    float actual = parse<float>("+inf");
    assert(isinf(actual) && actual > 0 && "test_parse_positive_infinity_explicit_float failed");
}

TEST(test_parse_negative_infinity_float) {
    float actual = parse<float>("-inf");
    assert(isinf(actual) && actual < 0 && "test_parse_negative_infinity_float failed");
}

TEST(test_parse_nan_float) {
    float actual = parse<float>("nan");
    assert(isnan(actual) && "test_parse_nan_float failed");
}

TEST(test_parse_infinity_invalid_int) {
    bool thrown = false;
    try {
        parse<int>("inf");
    } catch (const exception&) {
        thrown = true;
    }
    assert(thrown && "test_parse_infinity_invalid_int failed");
}

TEST(test_parse_nan_invalid_int) {
    bool thrown = false;
    try {
        parse<int>("nan");
    } catch (const exception&) {
        thrown = true;
    }
    assert(thrown && "test_parse_nan_invalid_int failed");
}

#endif
