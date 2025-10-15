#pragma once

#include <string>
#include <filesystem>

using namespace std;

namespace fs = filesystem;

string get_filename_only(const string& fname) {
    return fs::path(fname).stem().string();
}

#ifdef TEST

TEST(test_get_filename_only_basic) {
    string actual = get_filename_only("path/to/file.txt");
    assert(actual == "file" && "Basic case with extension");
}

TEST(test_get_filename_only_no_extension) {
    string actual = get_filename_only("path/to/filename");
    assert(actual == "filename" && "Filename without extension");
}

TEST(test_get_filename_only_hidden_file) {
    string actual = get_filename_only(".config");
    assert(actual == ".config" && "Hidden file without extension");
}

TEST(test_get_filename_only_hidden_file_with_extension) {
    string actual = get_filename_only("path/.hidden.conf");
    assert(actual == ".hidden" && "Hidden file with extension");
}

TEST(test_get_filename_only_multiple_dots) {
    string actual = get_filename_only("archive.tar.gz");
    assert(actual == "archive.tar" && "Multiple dots in filename");
}

TEST(test_get_filename_only_just_filename) {
    string actual = get_filename_only("myfile.cpp");
    assert(actual == "myfile" && "Just filename with extension");
}

TEST(test_get_filename_only_empty_string) {
    string actual = get_filename_only("");
    assert(actual == "" && "Empty input string");
}

TEST(test_get_filename_only_directory_path) {
    string actual = get_filename_only("path/to/directory/");
    // Behavior of stem() on directory paths can be platform-dependent or empty
    // Let's assert it's empty as a common case.
    assert(actual == "" && "Directory path ending with slash");
}

TEST(test_get_filename_only_root_path) {
    string actual = get_filename_only("/");
    assert(actual == "" && "Root path");
}

TEST(test_get_filename_only_dot_filename) {
    string actual = get_filename_only(".");
    assert(actual == "." && "Filename is just a dot");
}

TEST(test_get_filename_only_double_dot_filename) {
    string actual = get_filename_only("..");
    assert(actual == ".." && "Filename is double dot");
}

#endif
