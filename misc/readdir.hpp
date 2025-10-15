#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <stdexcept>

#include "ERROR.hpp"
#include "array_merge.hpp"

using namespace std;

namespace fs = filesystem;

vector<string> readdir(const string& dir, const string& pattern = "", bool recursive = true) {
    vector<string> result;

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        throw ERROR("Directory does not exist or is inaccessible: " + dir);
    }

    // Local lambda to process directory entries
    auto process = [&](const auto& begin, const auto& end) {
        for (auto it = begin; it != end; ++it) {
            if (it->is_regular_file()) {
                string path = it->path().string();
                if (pattern.empty()) {
                    result.push_back(path);
                }
                else if (pattern[0] == '*' && it->path().extension() == pattern.substr(1)) {
                    result.push_back(path);
                }
            }
        }
    };

    try {
        if (recursive) {
            fs::recursive_directory_iterator iterator(dir, fs::directory_options::skip_permission_denied);
            process(iterator, fs::recursive_directory_iterator());
        } else {
            fs::directory_iterator iterator(dir, fs::directory_options::skip_permission_denied);
            process(iterator, fs::directory_iterator());
        }
    } catch (const fs::filesystem_error& e) {
        throw ERROR("Error reading directory: " + string(e.what()));
    }

    return result;
}

vector<string> readdir(const string& dir, const vector<string>& patterns, bool recursive = true) {
    vector<string> result;
    for (const string& pattern: patterns)
        result = array_merge(result, readdir(dir, pattern, recursive));
    return result;
}

#ifdef TEST

#include <fstream>

// Helper to create test directory structure
void test_readir_setup_test_directory(const string& root) {
    fs::create_directory(root);
    fs::create_directory(root + "/subdir1");
    fs::create_directory(root + "/subdir1/subsubdir");
    ofstream(root + "/file1.txt").close();
    ofstream(root + "/file2.jpg").close();
    ofstream(root + "/subdir1/file3.txt").close();
    ofstream(root + "/subdir1/subsubdir/file4.txt").close();
}

// Helper to clean up test directory
void test_readir_cleanup_test_directory(const string& root) {
    fs::remove_all(root);
}

// Test cases
TEST(test_readdir_non_recursive_no_pattern) {
    string test_dir = "test_dir";
    test_readir_setup_test_directory(test_dir);

    auto files = readdir(test_dir, "", false);
    assert(files.size() == 2); // file1.txt, file2.jpg
    assert(find(files.begin(), files.end(), test_dir + "/file1.txt") != files.end());
    assert(find(files.begin(), files.end(), test_dir + "/file2.jpg") != files.end());

    test_readir_cleanup_test_directory(test_dir);
}

TEST(test_readdir_recursive_no_pattern) {
    string test_dir = "test_dir";
    test_readir_setup_test_directory(test_dir);

    auto files = readdir(test_dir, "", true);
    assert(files.size() == 4); // file1.txt, file2.jpg, subdir1/file3.txt, subdir1/subsubdir/file4.txt
    assert(find(files.begin(), files.end(), test_dir + "/file1.txt") != files.end());
    assert(find(files.begin(), files.end(), test_dir + "/file2.jpg") != files.end());
    assert(find(files.begin(), files.end(), test_dir + "/subdir1/file3.txt") != files.end());
    assert(find(files.begin(), files.end(), test_dir + "/subdir1/subsubdir/file4.txt") != files.end());

    test_readir_cleanup_test_directory(test_dir);
}

TEST(test_readdir_non_recursive_with_pattern) {
    string test_dir = "test_dir";
    test_readir_setup_test_directory(test_dir);

    auto files = readdir(test_dir, "*.txt", false);
    assert(files.size() == 1); // Only file1.txt
    assert(find(files.begin(), files.end(), test_dir + "/file1.txt") != files.end());
    assert(find(files.begin(), files.end(), test_dir + "/file2.jpg") == files.end());

    test_readir_cleanup_test_directory(test_dir);
}

TEST(test_readdir_recursive_with_pattern) {
    string test_dir = "test_dir";
    test_readir_setup_test_directory(test_dir);

    auto files = readdir(test_dir, "*.txt", true);
    assert(files.size() == 3); // file1.txt, subdir1/file3.txt, subdir1/subsubdir/file4.txt
    assert(find(files.begin(), files.end(), test_dir + "/file1.txt") != files.end());
    assert(find(files.begin(), files.end(), test_dir + "/subdir1/file3.txt") != files.end());
    assert(find(files.begin(), files.end(), test_dir + "/subdir1/subsubdir/file4.txt") != files.end());
    assert(find(files.begin(), files.end(), test_dir + "/file2.jpg") == files.end());

    test_readir_cleanup_test_directory(test_dir);
}

TEST(test_readdir_empty_directory) {
    string test_dir = "test_dir";
    fs::create_directory(test_dir);

    auto files = readdir(test_dir, "", true);
    assert(files.empty());

    test_readir_cleanup_test_directory(test_dir);
}

TEST(test_readdir_invalid_directory) {
    string test_dir = "test_dir";
    bool threw_exception = false;
    try {
        readdir(test_dir + "/nonexistent");
    } catch (const exception&) {
        threw_exception = true;
    }
    assert(threw_exception);

    // No cleanup needed (directory doesn't exist)
}

#endif