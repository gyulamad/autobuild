#pragma once

#include <vector>

using namespace std;

template<typename T>
vector<T> array_slice(const vector<T>& original, size_t offset, size_t length) {
    if (offset >= original.size()) return {}; // Offset out of bounds -> empty result

    size_t end = offset + length;
    if (end > original.size()) end = original.size(); // Clamp to vector size

    return vector<T>(original.begin() + offset, original.begin() + end);
}

template<typename T>
vector<T> array_slice(const vector<T>& original, size_t offset) {
    return array_slice(original, offset, original.size() - offset);
}

#ifdef TEST

TEST(test_array_slice_basic) {
    vector<int> v = {10, 20, 30, 40, 50};
    vector<int> sliced = array_slice(v, 2);
    assert(sliced == vector<int>({30, 40, 50}));
}

TEST(test_array_slice_offset_out_of_bounds) {
    vector<int> v = {1, 2, 3};
    vector<int> sliced = array_slice(v, 5);
    assert(sliced.empty());
}

TEST(test_array_slice_offset_and_length) {
    vector<int> v = {10, 20, 30, 40, 50};
    vector<int> sliced = array_slice(v, 1, 3);
    assert(sliced == vector<int>({20, 30, 40}));
}

TEST(test_array_slice_offset_and_too_long_length) {
    vector<int> v = {10, 20, 30};
    vector<int> sliced = array_slice(v, 1, 10);
    assert(sliced == vector<int>({20, 30}));
}

#endif