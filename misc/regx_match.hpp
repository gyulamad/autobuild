#pragma once

#include <string>
#include <vector>

// NOTE: Do not compiles with -Ofast + -fsanitize=address
//       or use: __attribute__((optimize("O0")))
//       or: #pragma GCC optimize("O0")
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105562#c27
#include <regex>

using namespace std;
using namespace regex_constants;

#pragma GCC optimize("O0")

__attribute__((optimize("O0")))
int regx_match(
    const string& pattern, 
    const string& str, 
    vector<string>* matches = nullptr,
    syntax_option_type flags = ECMAScript  // Add flags parameter
) {
    regex r(pattern, flags);
    smatch m;
    if (regex_search(str, m, r)) {
        if (matches != nullptr) {
            // Clear the vector before adding more matches
            matches->clear();
            for (unsigned int i = 0; i < m.size(); i++)
                matches->push_back(m[i].str());
        }
        return 1;
    }
    return 0;
}

#pragma GCC reset_options

#ifdef TEST

TEST(test_regx_match_basic) {
    string pattern = "hello";
    string str = "hello world";
    int result = regx_match(pattern, str);
    assert(result == 1 && "Basic match");
}

TEST(test_regx_match_no_match) {
    string pattern = "goodbye";
    string str = "hello world";
    int result = regx_match(pattern, str);
    assert(result == 0 && "No match");
}

TEST(test_regx_match_capture_groups) {
    string pattern = "(hello) (world)";
    string str = "hello world";
    vector<string> matches;
    int result = regx_match(pattern, str, &matches);
    assert(result == 1 && "Match with capture groups");
    assert(matches.size() == 3 && "Capture group count");
    assert(matches[0] == "hello world" && "Full match");
    assert(matches[1] == "hello" && "First capture group");
    assert(matches[2] == "world" && "Second capture group");
}

TEST(test_regx_match_empty_pattern) {
    string pattern = "";
    string str = "hello world";
    int result = regx_match(pattern, str);
    assert(result == 1 && "Empty pattern (matches any string)");
}

TEST(test_regx_match_empty_string) {
    string pattern = "hello";
    string str = "";
    int result = regx_match(pattern, str);
    assert(result == 0 && "Empty string");
}

TEST(test_regx_match_special_characters) {
    string pattern = "\\d+"; // Matches one or more digits
    string str = "123abc";
    int result = regx_match(pattern, str);
    assert(result == 1 && "Special characters (digits)");
}

// Test case-insensitive match with multiple matches
TEST(test_regx_match_case_insensitive_multiple_matches) {
    string pattern = "hello";
    string str = "HELLO hello HeLlO";
    vector<string> matches;
    int result = regx_match(pattern, str, &matches, regex_constants::icase);
    assert(result == 1 && "test_regx_match_case_insensitive_multiple_matches failed");
    assert(matches.size() == 1 && matches[0] == "HELLO" && "First match incorrect");
}

// Test case-insensitive match with regex_constants::icase
TEST(test_regx_match_case_insensitive_with_flag) {
    string pattern = "WORLD";
    string str = "Hello World";
    int result = regx_match(pattern, str, nullptr, regex_constants::icase);
    assert(result == 1 && "test_regx_match_case_insensitive_with_flag failed");
}

// Test case-sensitive match (default behavior)
TEST(test_regx_match_case_sensitive) {
    string pattern = "HELLO";
    string str = "hello world";
    int result = regx_match(pattern, str);  // No flag (case-sensitive)
    assert(result == 0 && "test_regx_match_case_sensitive failed");
}

TEST(test_regx_match_case_insensitive) {
    string pattern = "HELLO";
    string str = "hello world";
    int result = regx_match(pattern, str, nullptr, regex_constants::icase);  // Pass icase flag
    assert(result == 1 && "test_regx_match_case_insensitive failed");
}

TEST(test_regx_match_multiple_matches) {
    string pattern = "\\b\\w+\\b"; // Matches whole words
    string str = "hello world";
    vector<string> matches;
    int result = regx_match(pattern, str, &matches);
    assert(result == 1 && "Multiple matches");
    assert(matches.size() == 1 && "First match only (regex_search behavior)");
    assert(matches[0] == "hello" && "First match");
}

// Test invalid regex pattern
TEST(test_regx_match_invalid_pattern) {
    bool thrown = false;
    try {
        string pattern = "(?i)HELLO";  // Invalid syntax for ECMAScript
        regx_match(pattern, "hello world");
    } catch (const regex_error&) {
        thrown = true;
    }
    assert(thrown && "test_regx_match_invalid_pattern failed");
}

#endif
