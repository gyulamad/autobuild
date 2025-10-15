#pragma once

#include <vector>
#include <string>

using namespace std;

template<typename S>
vector<S> str_serialize(const string& str) {
    vector<S> serialized;
    size_t size = str.size();
    // Serialize size as 4 bytes (big-endian)
    serialized.push_back(static_cast<S>((size >> 24) & 0xFF));
    serialized.push_back(static_cast<S>((size >> 16) & 0xFF));
    serialized.push_back(static_cast<S>((size >> 8) & 0xFF));
    serialized.push_back(static_cast<S>(size & 0xFF));
    for (char c : str)
        serialized.push_back(static_cast<S>(c));
    return serialized;
}

#ifdef TEST

#include "str_deserialize.hpp"

// Test struct for organizing test data
struct test_str_serialize_deserialize_TestData {
    string input;
};

// Test case: Serialize and deserialize a regular string
TEST(test_str_serialize_deserialize_valid_string) {
    test_str_serialize_deserialize_TestData data = {"Hello, World!"};
    vector<char> serialized = str_serialize<char>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<char>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized string should match original");
}

// Test case: Serialize and deserialize an empty string
TEST(test_str_serialize_deserialize_empty_string) {
    test_str_serialize_deserialize_TestData data = {""};
    vector<char> serialized = str_serialize<char>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<char>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized empty string should match original");
}

// Test case: Serialize and deserialize a string with special characters
TEST(test_str_serialize_deserialize_special_characters) {
    test_str_serialize_deserialize_TestData data = {"@#$%^&*()\n\t"};
    vector<char> serialized = str_serialize<char>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<char>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized string with special characters should match original");
}


// Test case: Serialize and deserialize a regular string
TEST(test_str_serialize_deserialize_valid_string_float) {
    test_str_serialize_deserialize_TestData data = {"Hello, World!"};
    vector<float> serialized = str_serialize<float>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<float>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized string should match original");
}

// Test case: Serialize and deserialize an empty string
TEST(test_str_serialize_deserialize_empty_string_float) {
    test_str_serialize_deserialize_TestData data = {""};
    vector<float> serialized = str_serialize<float>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<float>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized empty string should match original");
}

// Test case: Serialize and deserialize a string with special characters
TEST(test_str_serialize_deserialize_special_characters_float) {
    test_str_serialize_deserialize_TestData data = {"@#$%^&*()\n\t"};
    vector<float> serialized = str_serialize<float>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<float>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized string with special characters should match original");
}


// Test case: Serialize and deserialize a regular string
TEST(test_str_serialize_deserialize_valid_string_long_double) {
    test_str_serialize_deserialize_TestData data = {"Hello, World!"};
    vector<long double> serialized = str_serialize<long double>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<long double>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized string should match original");
}

// Test case: Serialize and deserialize an empty string
TEST(test_str_serialize_deserialize_empty_string_long_double) {
    test_str_serialize_deserialize_TestData data = {""};
    vector<long double> serialized = str_serialize<long double>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<long double>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized empty string should match original");
}

// Test case: Serialize and deserialize a string with special characters
TEST(test_str_serialize_deserialize_special_characters_long_double) {
    test_str_serialize_deserialize_TestData data = {"@#$%^&*()\n\t"};
    vector<long double> serialized = str_serialize<long double>(data.input);
    size_t nxt = 0;
    string deserialized = str_deserialize<long double>(serialized, nxt);
    assert(deserialized == data.input && "Deserialized string with special characters should match original");
}

#endif