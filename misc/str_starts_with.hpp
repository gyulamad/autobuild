#pragma once

#include <string>

using namespace std;

bool str_starts_with(const string& str, const string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}
    

#ifdef TEST

TEST(test_str_starts_with_basic) {
    string str = "hello world";
    string prefix = "hello";
    bool actual = str_starts_with(str, prefix);
    bool expected = true;
    assert(actual == expected && "Basic starts_with");
}

TEST(test_str_starts_with_empty_prefix) {
    string str = "hello world";
    string prefix = "";
    bool actual = str_starts_with(str, prefix);
    bool expected = true;
    assert(actual == expected && "Empty prefix");
}

TEST(test_str_starts_with_empty_string) {
    string str = "";
    string prefix = "hello";
    bool actual = str_starts_with(str, prefix);
    bool expected = false;
    assert(actual == expected && "Empty string");
}

TEST(test_str_starts_with_prefix_longer_than_string) {
    string str = "hello";
    string prefix = "hello world";
    bool actual = str_starts_with(str, prefix);
    bool expected = false;
    assert(actual == expected && "Prefix longer than string");
}

TEST(test_str_starts_with_no_match) {
    string str = "hello world";
    string prefix = "world";
    bool actual = str_starts_with(str, prefix);
    bool expected = false;
    assert(actual == expected && "No match");
}

TEST(test_str_starts_with_case_sensitive) {
    string str = "Hello World";
    string prefix = "hello";
    bool actual = str_starts_with(str, prefix);
    bool expected = false;
    assert(actual == expected && "Case sensitive");
}

#endif
