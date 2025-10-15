#pragma once

#include <vector>
#include <algorithm>

using namespace std;

template<typename T>
void vector_remove(vector<T>& vec, const T& value) {
    vec.erase(remove(vec.begin(), vec.end(), value), vec.end());
}

#ifdef TEST

TEST(test_vector_remove) {
    vector<string> v = {"apple", "banana", "cherry", "banana"};
    vector_remove(v, string("banana"));
    assert(v.size() == 2);
    assert(v[0] == "apple");
    assert(v[1] == "cherry");
}

TEST(test_vector_remove_not_found) {
    vector<int> v = {1, 2, 3, 4};
    vector_remove(v, 5);
    assert(v.size() == 4);
    assert(v[0] == 1);
    assert(v[1] == 2);
    assert(v[2] == 3);
    assert(v[3] == 4);
}

TEST(test_vector_remove_empty) {
    vector<int> v;
    vector_remove(v, 1);
    assert(v.empty());
}

TEST(test_vector_remove_all_elements) {
    vector<int> v = {7, 7, 7, 7};
    vector_remove(v, 7);
    assert(v.empty());
}

TEST(test_vector_remove_one_occurrence) {
    vector<int> v = {1, 2, 3, 4};
    vector_remove(v, 3);
    assert(v.size() == 3);
    assert(v[0] == 1);
    assert(v[1] == 2);
    assert(v[2] == 4);
}

#endif