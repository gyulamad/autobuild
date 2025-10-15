#pragma once

#include <string>

#include "remove_extension.hpp"
#include "str_starts_with.hpp"

using namespace std;

string replace_extension(const string& fname, const string& newext) {
    return remove_extension(fname) + (!newext.empty() ? str_starts_with(newext, ".") ? newext : ("." + newext) : "");
}

#ifdef TEST
#include "str_contains.hpp"

// Tests for replace_extension
TEST(test_replace_extension_add_new_extension) {
    string filename = "document.txt";
    string newext = "bak";
    string result = replace_extension(filename, newext);
    assert(result == "document.bak" && "Should replace .txt with .bak");
}

TEST(test_replace_extension_with_dot_in_newext) {
    string filename = "document.txt";
    string newext = ".pdf";
    string result = replace_extension(filename, newext);
    assert(result == "document.pdf" && "Should replace .txt with .pdf");
}

TEST(test_replace_extension_empty_newext) {
    string filename = "document.txt";
    string newext = "";
    string result = replace_extension(filename, newext);
    assert(result == "document" && "Should remove extension when newext is empty");
}

TEST(test_replace_extension_no_original_extension) {
    string filename = "document";
    string newext = "doc";
    string result = replace_extension(filename, newext);
    assert(result == "document.doc" && "Should add .doc to filename with no extension");
}

TEST(test_replace_extension_multiple_dots) {
    string filename = "my.file.name.txt";
    string newext = "jpg";
    string result = replace_extension(filename, newext);
    assert(result == "my.file.name.jpg" && "Should replace .txt with .jpg");
}

TEST(test_replace_extension_empty_filename) {
    string filename = "";
    string newext = "txt";
    string result = replace_extension(filename, newext);
    assert(result == ".txt" && "Should return .txt for empty filename");
}

TEST(test_replace_extension_only_extension) {
    string filename = ".txt";
    string newext = "bak";
    string result = replace_extension(filename, newext);
    assert(result == ".bak" && "Should replace .txt with .bak for filename with only extension");
}

TEST(test_replace_extension_only_with_path) {
    string filename = "path/to/foo.txt";
    string newext = "bak";
    string result = replace_extension(filename, newext);
    assert(result == "path/to/foo.bak" && "Should replace .txt with .bak for filename with only extension");
}

#endif