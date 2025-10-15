#pragma once

#include <string>
#include <vector>

#include "ERROR.hpp"

using namespace std;

vector<string> explode(const string& delimiter, const string& str) {
    if (delimiter.empty())
        throw ERROR("Delimeter can not be empty.");
    vector<string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    // Split the string by the delimiter
    while (end != string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    // Add the last part of the string
    result.push_back(str.substr(start));

    return result;
}

#ifdef TEST

TEST(test_explode_basic) {
    auto parts = explode(",", "a,b,c");
    assert(parts.size() == 3 && "Basic split count");
    assert(parts[0] == "a" && parts[1] == "b" && parts[2] == "c");
}

TEST(test_explode_no_delimiter) {
    auto parts = explode("-", "test");
    assert(parts.size() == 1 && "No delimiter found");
    assert(parts[0] == "test");
}

TEST(test_explode_start_with_delimiter) {
    auto parts = explode("-", "-apples");
    assert(parts.size() == 2 && "Start with delimiter");
    assert(parts[0].empty() && parts[1] == "apples");
}

TEST(test_explode_end_with_delimiter) {
    auto parts = explode("-", "apples-");
    assert(parts.size() == 2 && "End with delimiter");
    assert(parts[0] == "apples" && parts[1].empty());
}

TEST(test_explode_consecutive_delimiters) {
    auto parts = explode(",", "a,,b");
    assert(parts.size() == 3 && "Consecutive delimiters");
    assert(parts[0] == "a" && parts[1].empty() && parts[2] == "b");
}

TEST(test_explode_only_delimiters) {
    auto parts = explode("--", "----");
    assert(parts.size() == 3 && "Only delimiters");
    assert(parts[0].empty() && parts[1].empty() && parts[2].empty());
}

TEST(test_explode_empty_input) {
    auto parts = explode(",", "");
    assert(parts.size() == 1 && "Empty input");
    assert(parts[0].empty());
}

TEST(test_explode_invalid_delimiter) {
    bool exception_thrown = false;
    try { explode("", "test"); }
    catch (...) { exception_thrown = true; }
    assert(exception_thrown && "Empty delimiter exception");
}

#endif
