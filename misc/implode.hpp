#pragma once

#include <string>
#include <vector>
#include <sstream>

using namespace std;

string implode(const string& delimiter, const vector<string>& elements) {
    ostringstream oss;
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i != 0) oss << delimiter;
        oss << elements[i];
    }
    return oss.str();
}
    
#ifdef TEST

TEST(test_implode_basic) {
    vector<string> parts = {"a", "b", "c"};
    string result = implode(",", parts);
    assert(result == "a,b,c" && "Basic implode");
}

TEST(test_implode_single_element) {
    vector<string> parts = {"hello"};
    string result = implode("-", parts);
    assert(result == "hello" && "Single element");
}

TEST(test_implode_empty_elements) {
    vector<string> parts = {"", "", ""};
    string result = implode(":", parts);
    assert(result == "::" && "Empty elements");
}

TEST(test_implode_empty_delimiter) {
    vector<string> parts = {"a", "b", "c"};
    string result = implode("", parts);
    assert(result == "abc" && "Empty delimiter");
}

TEST(test_implode_empty_vector) {
    vector<string> parts = {};
    string result = implode(",", parts);
    assert(result.empty() && "Empty vector");
}

TEST(test_implode_mixed_elements) {
    vector<string> parts = {"a", "", "c"};
    string result = implode("-", parts);
    assert(result == "a--c" && "Mixed elements");
}

#endif
