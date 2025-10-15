#pragma once

#include <string>
#include <filesystem>

#include "fix_path.hpp"

using namespace std;

namespace fs = filesystem;

string get_absolute_path(const string& fname) {
    return fix_path(fs::absolute(fs::path(fname)).string());
}
 

#ifdef TEST

#include "get_cwd.hpp"

TEST(test_get_absolute_path_relative) {
    string cwd = get_cwd();
    string actual = get_absolute_path("some/relative/path.txt");
    string expected = cwd + "/some/relative/path.txt";
    assert(actual == expected && "Relative path should become absolute");
}

TEST(test_get_absolute_path_relative_with_dots) {
    string cwd = get_cwd();
    string actual = get_absolute_path("some/../other/./file.dat");
    string expected = cwd + "/other/file.dat"; // fs::absolute resolves . and ..
    assert(actual == expected && "Relative path with dots should be resolved");
}

TEST(test_get_absolute_path_already_absolute) {
    // Assuming /tmp exists and is a valid absolute path start on Linux
    string actual = get_absolute_path("/tmp/absolute/file.log");
    string expected = "/tmp/absolute/file.log"; // Should remain absolute and fixed
    assert(actual == expected && "Already absolute path should remain absolute");
}

TEST(test_get_absolute_path_with_trailing_slash) {
    string cwd = get_cwd();
    string actual = get_absolute_path("some/dir/");
    string expected = cwd + "/some/dir"; // fix_path removes trailing slash
    assert(actual == expected && "Trailing slash should be removed");
}

TEST(test_get_absolute_path_root) {
    string actual = get_absolute_path("/");
    string expected = "/"; // Root path should remain "/"
    assert(actual == expected && "Root path should be handled correctly");
}

TEST(test_get_absolute_path_empty) {
    bool threw = false;
    try {
        get_absolute_path("");
    } catch (exception& e) {
        threw = true;
        assert(str_contains(e.what(), "filesystem error: cannot make absolute path: Invalid argument []"));
    }
    assert(threw && "Empty path should throw");
}

TEST(test_get_absolute_path_dot) {
    string cwd = get_cwd();
    string actual = get_absolute_path(".");
    string expected = cwd; // "." resolves to current working directory
    assert(actual == expected && "'.' should resolve to current working directory");
}

TEST(test_get_absolute_path_double_dot) {
    string cwd = get_cwd();
    // Get parent path for expectation
    string parent_path = fix_path(fs::path(cwd).parent_path().string());
    string actual = get_absolute_path("..");
    string expected = parent_path; // ".." resolves to parent directory
    assert(actual == expected && "'..' should resolve to parent directory");
}

#endif
