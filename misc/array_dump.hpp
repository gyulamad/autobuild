#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "implode.hpp"
#include "capture_cout.hpp"

using namespace std;

template<typename T>
string array_dump(const vector<T>& vec, bool show = false) {
    vector<string> dump;
    for (const T& elem : vec) {
        stringstream ss;
        ss << elem;
        dump.push_back(ss.str());
    }

    string output = "vector(" + to_string(vec.size()) + "): (" + implode(", ", dump) + ")";
    if (show) cout << output << endl;
    return output;
}
    

#ifdef TEST

TEST(test_array_dump_basic) {
    vector<int> vec = {1, 2, 3};
    string expected = 
        "dump vector(3):\n"
        "0: 1\n"
        "1: 2\n"
        "2: 3\n";
    
    string result;
    string output = capture_cout([&]() { result = array_dump(vec, true); });
    assert(output == expected && "Basic dump shows");
    assert(result == expected && "Basic dump returns");
}

TEST(test_array_dump_empty_vector) {
    vector<int> vec;
    string expected = 
        "dump vector(0):\n";
    
    string output = capture_cout([&]() { array_dump(vec, true); });
    assert(output == expected && "Empty vector dump");
}

TEST(test_array_dump_strings) {
    vector<string> vec = {"hello", "world"};
    string expected = 
        "dump vector(2):\n"
        "0: hello\n"
        "1: world\n";
    
    string output = capture_cout([&]() { array_dump(vec, true); });
    assert(output == expected && "Strings dump");
}

TEST(test_array_dump_no_debug) {
    vector<int> vec = {1, 2, 3};
    string expected = 
        "0: 1\n"
        "1: 2\n"
        "2: 3\n";
    
    string results;
    string output = capture_cout([&]() { results = array_dump(vec, false); });
    assert(output.empty() && "No output dump");
    assert(results == expected && "Still returns dump even if not shown");
}

TEST(test_array_dump_basic);
TEST(test_array_dump_empty_vector);
TEST(test_array_dump_strings);
TEST(test_array_dump_no_debug);
#endif
