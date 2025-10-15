#pragma once

#include <vector>
#include "vector_concat.hpp"

using namespace std;

template<typename S, typename T>
void Serializable_vector_serialize(vector<S>& serialized, vector<T>& vec) {
    serialized.push_back(vec.size());
    for (T& v: vec)
        serialized = vector_concat(serialized, v.serialize());
}


#ifdef TEST

#include "str_contains.hpp"

// Test class with serialize and deserialize methods
class test_Serializable_vector_serialize_deserialize_TestType {
public:
    test_Serializable_vector_serialize_deserialize_TestType() = default;
    explicit test_Serializable_vector_serialize_deserialize_TestType(const string& value) : value(value) {}

    vector<char> serialize() const {
        return str_serialize<char>(value);
    }

    void deserialize(const vector<char>& serialized, size_t& nxt) {
        value = str_deserialize<char>(serialized, nxt);
    }

    bool operator==(const test_Serializable_vector_serialize_deserialize_TestType& other) const {
        return value == other.value;
    }

private:
    string value; // gamelCase for member variable
};

// Test struct for organizing test data
struct test_Serializable_vector_serialize_deserialize_TestData {
    vector<test_Serializable_vector_serialize_deserialize_TestType> input;
};

// Test struct to hold test data
struct test_Serializable_vector_serialize_deserialize_TestData2 {
    vector<int> input;           // Original vector to serialize
    vector<size_t> serialized;   // Expected serialized output
    vector<int> deserialized;    // Deserialized result
};

// Mock serialization/deserialization for int (since T needs serialize/deserialize)
struct test_Serializable_vector_serialize_deserialize_IntWrapper {
    int value;
    
    vector<size_t> serialize() {
        return {static_cast<size_t>(value)};
    }
    
    void deserialize(const vector<size_t>& serialized, size_t& nxt) {
        if (nxt >= serialized.size()) 
            throw ERROR("test_Serializable_vector_serialize_deserialize_IntWrapper: out of bounds");
        value = static_cast<int>(serialized[nxt++]);
    }
};

// Test case: Serialize and deserialize a vector with multiple elements
TEST(test_Serializable_vector_serialize_deserialize_multiple_elements) {
    test_Serializable_vector_serialize_deserialize_TestData data = {
        {test_Serializable_vector_serialize_deserialize_TestType("apple"), test_Serializable_vector_serialize_deserialize_TestType("banana"), test_Serializable_vector_serialize_deserialize_TestType("cherry")}
    };
    vector<char> serialized;
    Serializable_vector_serialize(serialized, data.input);
    vector<test_Serializable_vector_serialize_deserialize_TestType> deserialized;
    test_Serializable_vector_serialize_deserialize_TestType temp;
    size_t nxt = 0;
    Serializable_vector_deserialize(temp, deserialized, serialized, nxt);
    assert(deserialized.size() == data.input.size() && "Deserialized vector size should match original");
    for (size_t i = 0; i < deserialized.size(); i++)
        assert(deserialized[i] == data.input[i] && "Deserialized elements should match original");
}

// Test case: Serialize and deserialize an empty vector
TEST(test_Serializable_vector_serialize_deserialize_empty_vector) {
    test_Serializable_vector_serialize_deserialize_TestData data = {{}};
    vector<char> serialized;
    Serializable_vector_serialize(serialized, data.input);
    vector<test_Serializable_vector_serialize_deserialize_TestType> deserialized;
    test_Serializable_vector_serialize_deserialize_TestType temp;
    size_t nxt = 0;
    Serializable_vector_deserialize(temp, deserialized, serialized, nxt);
    assert(deserialized.empty() && "Deserialized empty vector should be empty");
}

// Test case: Serialize and deserialize a vector with a single element
TEST(test_Serializable_vector_serialize_deserialize_single_element) {
    test_Serializable_vector_serialize_deserialize_TestData data = {{test_Serializable_vector_serialize_deserialize_TestType("solo")}};
    vector<char> serialized;
    Serializable_vector_serialize(serialized, data.input);
    vector<test_Serializable_vector_serialize_deserialize_TestType> deserialized;
    test_Serializable_vector_serialize_deserialize_TestType temp;
    size_t nxt = 0;
    Serializable_vector_deserialize(temp, deserialized, serialized, nxt);
    assert(deserialized.size() == 1 && "Deserialized vector should have one element");
    assert(deserialized[0] == data.input[0] && "Deserialized single element should match original");
}

TEST(test_Serializable_vector_serialize_deserialize_valid_input) {
    // Setup test data
    test_Serializable_vector_serialize_deserialize_TestData2 data;
    data.input = {1, 2, 3};

    // Serialize
    vector<size_t> serialized;
    vector<test_Serializable_vector_serialize_deserialize_IntWrapper> input_wrapped;
    for (int v : data.input) input_wrapped.push_back({v});
    Serializable_vector_serialize(serialized, input_wrapped);

    // Deserialize
    vector<test_Serializable_vector_serialize_deserialize_IntWrapper> deserialized_wrapped;
    test_Serializable_vector_serialize_deserialize_IntWrapper wrapper;
    size_t nxt = 0;
    try {
        Serializable_vector_deserialize(wrapper, deserialized_wrapped, serialized, nxt);
    } catch (const exception& e) {
        assert(false && "Deserialization should not throw for valid input");
    }

    // Convert back to vector<int> for comparison
    data.deserialized.clear();
    for (const auto& w : deserialized_wrapped) {
        data.deserialized.push_back(w.value);
    }

    // Assert equality
    assert(data.input.size() == data.deserialized.size() && "Deserialized vector size should match input");
    for (size_t i = 0; i < data.input.size(); i++)
        assert(data.input[i] == data.deserialized[i] && "Deserialized values should match input");
}

TEST(test_Serializable_vector_serialize_deserialize_empty_vector2) {
    // Setup test data for empty vector
    test_Serializable_vector_serialize_deserialize_TestData2 data;
    data.input = {};

    // Serialize
    vector<size_t> serialized;
    vector<test_Serializable_vector_serialize_deserialize_IntWrapper> input_wrapped;
    Serializable_vector_serialize(serialized, input_wrapped);

    // Deserialize
    vector<test_Serializable_vector_serialize_deserialize_IntWrapper> deserialized_wrapped;
    test_Serializable_vector_serialize_deserialize_IntWrapper wrapper;
    size_t nxt = 0;
    try {
        Serializable_vector_deserialize(wrapper, deserialized_wrapped, serialized, nxt);
    } catch (const exception& e) {
        assert(false && "Deserialization should not throw for empty input");
    }

    // Convert back to vector<int>
    data.deserialized.clear();
    for (const auto& w : deserialized_wrapped) {
        data.deserialized.push_back(w.value);
    }

    // Assert equality
    assert(data.deserialized.empty() && "Deserialized vector should be empty");
}

TEST(test_Serializable_vector_serialize_deserialize_invalid_serialized_data) {
    // Setup test data with invalid serialized data
    vector<size_t> serialized = {2, 1}; // Size = 2, but only 1 value
    vector<test_Serializable_vector_serialize_deserialize_IntWrapper> deserialized_wrapped;
    test_Serializable_vector_serialize_deserialize_IntWrapper wrapper;
    size_t nxt = 0;
    bool threw = false;

    // Attempt to deserialize
    try {
        Serializable_vector_deserialize(wrapper, deserialized_wrapped, serialized, nxt);
    } catch (const exception& e) {
        string what = e.what();
        assert(str_contains(what, "test_Serializable_vector_serialize_deserialize_IntWrapper: out of bounds") && "Exception should contain 'Serialized size mismatch'");
        threw = true;
    }

    // Assert that an exception was thrown
    assert(threw && "Deserialization should throw for invalid serialized data");
}

#endif
