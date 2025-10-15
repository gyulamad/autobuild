#pragma once

#include <vector>

using namespace std;

template <typename T>
T array_shift(vector<T>& vec) {
    if (vec.empty())
        throw ERROR("Cannot shift from an empty vector");

    // Save the first element (to return it)
    T firstElement = std::move(vec.front());

    // Remove the first element
    vec.erase(vec.begin());

    // Return the shifted element
    return firstElement;
}
    

#ifdef TEST

TEST(test_array_shift_basic) {
    vector<int> vec = {1, 2, 3};
    int shifted = array_shift(vec);
    assert(shifted == 1 && "Shifted element");
    assert(vec.size() == 2 && "Vector size after shift");
    assert(vec[0] == 2 && "First element after shift");
    assert(vec[1] == 3 && "Second element after shift");
}

TEST(test_array_shift_single_element) {
    vector<int> vec = {42};
    int shifted = array_shift(vec);
    assert(shifted == 42 && "Shifted element");
    assert(vec.empty() && "Vector should be empty after shift");
}

TEST(test_array_shift_strings) {
    vector<string> vec = {"hello", "world"};
    string shifted = array_shift(vec);
    assert(shifted == "hello" && "Shifted element");
    assert(vec.size() == 1 && "Vector size after shift");
    assert(vec[0] == "world" && "First element after shift");
}

TEST(test_array_shift_empty_vector) {
    vector<int> vec;
    bool exception_thrown = false;
    try {
        array_shift(vec);
    } catch (const exception& e) {
        exception_thrown = true;
    }
    assert(exception_thrown && "Exception should be thrown for empty vector");
}

TEST(test_array_shift_move_semantics) {
    vector<string> vec = {"hello", "world"};
    string shifted = array_shift(vec);
    assert(shifted == "hello" && "Shifted element");
    assert(vec.size() == 1 && "Vector size after shift");
    assert(vec[0] == "world" && "First element after shift");
}

TEST(test_array_shift_large_vector) {
    vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int shifted = array_shift(vec);
    assert(shifted == 1 && "Shifted element");
    assert(vec.size() == 9 && "Vector size after shift");
    assert(vec[0] == 2 && "First element after shift");
    assert(vec[8] == 10 && "Last element after shift");
}

#endif
