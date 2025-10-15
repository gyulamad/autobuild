#pragma once

#include <string>
#include <vector>

#include "explode.hpp"
#include "implode.hpp"

using namespace std;

string remove_extension(const string& fname) {
    vector<string> splits = explode(".", fname);
    if (splits.size() > 1) splits.pop_back(); // remove last element but only if it had extension
    return implode(".", splits);
}

#ifdef TEST
#include "str_contains.hpp"

// Tests for remove_extension
TEST(test_remove_extension_single_extension) {
    string filename = "document.txt";
    string result = remove_extension(filename);
    assert(result == "document" && "Should remove .txt extension");
}

TEST(test_remove_extension_multiple_dots) {
    string filename = "my.file.name.txt";
    string result = remove_extension(filename);
    assert(result == "my.file.name" && "Should remove last extension .txt");
}

TEST(test_remove_extension_no_extension) {
    string filename = "document";
    string result = remove_extension(filename);
    assert(result == "document" && "Should return unchanged filename with no extension");
}

TEST(test_remove_extension_empty_string) {
    string filename = "";
    string result = remove_extension(filename);
    assert(result == "" && "Should return empty string for empty input");
}

TEST(test_remove_extension_only_extension) {
    string filename = ".txt";
    string result = remove_extension(filename);
    assert(result == "" && "Should return empty string for filename with only extension");
}

#endif