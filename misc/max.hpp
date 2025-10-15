#pragma once

#include <vector>

using namespace std;

template<typename T>
T max(const vector<T> values) {
    size_t size = values.size();
    if (size == 0)
        throw ERROR("Empty vector does not have max");
    T max = values[0];    
    for (size_t n = 1; n < size; n++) {
        T value = values[n];
        if (max < value) max = value;
    }
    return max;
}