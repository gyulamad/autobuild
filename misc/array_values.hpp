#pragma once

#include <vector>

using namespace std;

template<typename T>
auto array_values(const T& input_map) -> vector<typename T::mapped_type> {
    vector<typename T::mapped_type> result;
    result.reserve(input_map.size());
    
    for (const auto& pair : input_map) {
        result.push_back(pair.second);
    }
    
    return result;
}

#ifdef TEST

TEST(test_array_values_with_ordered_map_preserves_sorted_order) {
    map<string, int> input = {
        {"c", 3},
        {"a", 1}, 
        {"b", 2}
    };
    
    auto result = array_values(input);
    
    assert(result.size() == 3 && "Result should have 3 elements");
    // map sorts by key, so order should be: a=1, b=2, c=3
    assert(result[0] == 1 && "First element should be 1 (from key 'a')");
    assert(result[1] == 2 && "Second element should be 2 (from key 'b')");
    assert(result[2] == 3 && "Third element should be 3 (from key 'c')");
}

TEST(test_array_values_with_unordered_map_contains_all_values) {
    unordered_map<string, int> input = {
        {"foo", 123},
        {"bar", 888},
        {"bazz", 32}
    };
    
    auto result = array_values(input);
    
    assert(result.size() == 3 && "Result should have 3 elements");
    
    // Order is not guaranteed with unordered_map, so check if all values are present
    sort(result.begin(), result.end());
    vector<int> expected = {32, 123, 888};
    
    assert(result == expected && "Result should contain all expected values");
}

TEST(test_array_values_with_empty_map_returns_empty_vector) {
    map<string, int> empty_map;
    
    auto result = array_values(empty_map);
    
    assert(result.empty() && "Result should be empty for empty input");
    assert(result.size() == 0 && "Result size should be 0");
}

TEST(test_array_values_with_single_element_map) {
    map<string, double> single_map = {{"pi", 3.14159}};
    
    auto result = array_values(single_map);
    
    assert(result.size() == 1 && "Result should have 1 element");
    assert(result[0] == 3.14159 && "Result should contain the single value");
}

TEST(test_array_values_with_string_values) {
    map<string, string> string_map = {
        {"greeting", "hello"},
        {"farewell", "goodbye"},
        {"question", "what"}
    };
    
    auto result = array_values(string_map);
    
    assert(result.size() == 3 && "Result should have 3 elements");
    // map sorts by key: farewell, greeting, question
    assert(result[0] == "goodbye" && "First element should be 'goodbye'");
    assert(result[1] == "hello" && "Second element should be 'hello'");
    assert(result[2] == "what" && "Third element should be 'what'");
}

TEST(test_array_values_with_duplicate_values_preserves_all) {
    map<string, int> dup_map = {
        {"first", 42},
        {"second", 42},
        {"third", 99}
    };
    
    auto result = array_values(dup_map);
    
    assert(result.size() == 3 && "Result should have 3 elements");
    assert(result[0] == 42 && "First element should be 42");
    assert(result[1] == 42 && "Second element should be 42"); 
    assert(result[2] == 99 && "Third element should be 99");
}

#endif