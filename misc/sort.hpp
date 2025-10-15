#pragma once

#include <vector>

using namespace std;

template <typename T>
void sort(vector<T>& vec) {
    sort(vec.begin(), vec.end()); // Sorts in ascending order
}

#ifdef TEST

TEST(test_sort_basic) {
    vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};
    sort(vec);
    vector<int> expected = {1, 1, 2, 3, 4, 5, 6, 9};
    assert(vec == expected && "Basic sort (ascending)");
}

TEST(test_sort_empty_vector) {
    vector<int> vec;
    sort(vec);
    assert(vec.empty() && "Empty vector sort");
}

TEST(test_sort_single_element) {
    vector<int> vec = {42};
    sort(vec);
    vector<int> expected = {42};
    assert(vec == expected && "Single element sort");
}

TEST(test_sort_strings) {
    vector<string> vec = {"banana", "apple", "cherry"};
    sort(vec);
    vector<string> expected = {"apple", "banana", "cherry"};
    assert(vec == expected && "Sort strings (ascending)");
}

#endif
