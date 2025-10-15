#pragma once

#include <vector>

#include "foreach.hpp"
#include "in_array.hpp"

using namespace std;

// Function to merge two vectors
template <typename T>
vector<T> array_merge(const vector<T>& vec1, const vector<T>& vec2) {
    vector<T> results = vec1;
    foreach (vec2, [&](const T& item) { if (!in_array(item, results)) results.push_back(item); });
    return results;
}

#ifdef TEST

#include "vector_equal.hpp"

TEST(test_array_merge_basic) {
    vector<int> vec1 = {1, 2, 3};
    vector<int> vec2 = {4, 5, 6};
    vector<int> result = array_merge(vec1, vec2);
    vector<int> expected = {1, 2, 3, 4, 5, 6};
    assert(result == expected && "Basic merge");
}

TEST(test_array_merge_empty_first_vector) {
    vector<int> vec1;
    vector<int> vec2 = {4, 5, 6};
    vector<int> result = array_merge(vec1, vec2);
    vector<int> expected = {4, 5, 6};
    assert(result == expected && "Empty first vector");
}

TEST(test_array_merge_empty_second_vector) {
    vector<int> vec1 = {1, 2, 3};
    vector<int> vec2;
    vector<int> result = array_merge(vec1, vec2);
    vector<int> expected = {1, 2, 3};
    assert(result == expected && "Empty second vector");
}

TEST(test_array_merge_both_empty_vectors) {
    vector<int> vec1;
    vector<int> vec2;
    vector<int> result = array_merge(vec1, vec2);
    assert(result.empty() && "Both vectors empty");
}

TEST(test_array_merge_strings) {
    vector<string> vec1 = {"hello"};
    vector<string> vec2 = {"world"};
    vector<string> result = array_merge(vec1, vec2);
    vector<string> expected = {"hello", "world"};
    assert(result == expected && "Merge strings");
}

TEST(test_array_merge_large_vectors) {
    vector<int> vec1 = {1, 2, 3, 4, 5};
    vector<int> vec2 = {6, 7, 8, 9, 10};
    vector<int> result = array_merge(vec1, vec2);
    vector<int> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    assert(result == expected && "Merge large vectors");
}

TEST(test_array_merge_custom_objects) {
    struct Point {
        int x, y;
        bool operator==(const Point& other) const {
            return x == other.x && y == other.y;
        }
    };

    vector<Point> vec1 = {{1, 2}, {3, 4}};
    vector<Point> vec2 = {{5, 6}};
    vector<Point> result = array_merge(vec1, vec2);
    vector<Point> expected = {{1, 2}, {3, 4}, {5, 6}};
    assert(result == expected && "Merge custom objects");
}

// Test merging two empty vectors
TEST(test_array_merge_empty_vectors) {
    vector<string> vec1 = {};
    vector<string> vec2 = {};
    vector<string> actual = array_merge(vec1, vec2);
    vector<string> expected = {};
    assert(vector_equal(actual, expected) && "Merging two empty vectors should result in an empty vector");
}

// Test merging one empty and one non-empty vector
TEST(test_array_merge_empty_and_nonempty) {
    vector<string> vec1 = {};
    vector<string> vec2 = {"a", "b"};
    vector<string> actual = array_merge(vec1, vec2);
    vector<string> expected = {"a", "b"};
    assert(vector_equal(actual, expected) && "Merging empty with non-empty should return non-empty vector without duplicates");
}

// Test merging two vectors with no duplicates
TEST(test_array_merge_no_duplicates) {
    vector<string> vec1 = {"a", "b"};
    vector<string> vec2 = {"c", "d"};
    vector<string> actual = array_merge(vec1, vec2);
    vector<string> expected = {"a", "b", "c", "d"};
    assert(vector_equal(actual, expected) && "Merging vectors with no duplicates should combine all elements");
}

// Test merging vectors with some duplicates
TEST(test_array_merge_with_duplicates) {
    vector<string> vec1 = {"a", "b", "c"};
    vector<string> vec2 = {"b", "c", "d"};
    vector<string> actual = array_merge(vec1, vec2);
    vector<string> expected = {"a", "b", "c", "d"};
    assert(vector_equal(actual, expected) && "Merging vectors with duplicates should result in unique elements only");
}

// Test merging vectors where all elements are duplicates
TEST(test_array_merge_all_duplicates) {
    vector<string> vec1 = {"a", "b"};
    vector<string> vec2 = {"a", "b"};
    vector<string> actual = array_merge(vec1, vec2);
    vector<string> expected = {"a", "b"};
    assert(vector_equal(actual, expected) && "Merging vectors with all duplicates should result in unique elements only");
}

// Test merging with numeric type to ensure template works
TEST(test_array_merge_numeric_type) {
    vector<int> vec1 = {1, 2, 3};
    vector<int> vec2 = {2, 3, 4};
    vector<int> actual = array_merge(vec1, vec2);
    vector<int> expected = {1, 2, 3, 4};
    assert(vector_equal(actual, expected) && "Merging numeric vectors with duplicates should result in unique elements only");
}

#endif
