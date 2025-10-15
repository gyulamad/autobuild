#pragma once

#include <string>

using namespace std;

string str_cut_end(const string& s, size_t maxch = 300, const string& append = "...") {
    // Check if the string is already shorter than or equal to the limit
    if (s.length() <= maxch) return s;

    // Handle the case where maxch is smaller than or equal to the length of append
    if (maxch <= append.length()) return append;

    // Truncate the string and append the suffix
    return s.substr(0, maxch - append.length()) + append;
}
    
#ifdef TEST

// Test cases for str_cut_end
TEST(test_str_cut_end_when_string_is_short) {
    string input = "hello";
    string expected = "hello";
    string actual = str_cut_end(input, 10);
    assert(actual == expected && "test_str_cut_end_when_string_is_short failed");
}

TEST(test_str_cut_end_when_string_is_exact_length) {
    string input = "abcdefghij";
    string expected = "abcdefghij";
    string actual = str_cut_end(input, 10);
    assert(actual == expected && "test_str_cut_end_when_string_is_exact_length failed");
}

TEST(test_str_cut_end_when_string_needs_truncation) {
    string input = "abcdefghijklmnopqrstuvwxyz";
    string expected = "abcdefghijkl...";
    string actual = str_cut_end(input, 15);
    assert(actual == expected && "test_str_cut_end_when_string_needs_truncation failed");
}

TEST(test_str_cut_end_when_custom_append_is_used) {
    string input = "abcdefghijklmnopqrstuvwxyz";
    string expected = "abcdefghijklm!!";
    string actual = str_cut_end(input, 15, "!!");
    assert(actual == expected && "test_str_cut_end_when_custom_append_is_used failed");
}

TEST(test_str_cut_end_when_string_is_empty) {
    string input = "";
    string expected = "";
    string actual = str_cut_end(input, 10);
    assert(actual == expected && "test_str_cut_end_when_string_is_empty failed");
}

TEST(test_str_cut_end_when_maxch_is_less_than_append_length) {
    string input = "abcdefghijklmnopqrstuvwxyz";
    string expected = "...";
    string actual = str_cut_end(input, 2);
    assert(actual == expected && "test_str_cut_end_when_maxch_is_less_than_append_length failed");
}

#endif
