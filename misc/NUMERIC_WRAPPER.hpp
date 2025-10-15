#pragma once

#define NUMERIC_WRAPPER(T, v) \
    T& operator=(const T other) { v = other; return v; } \
    operator T() const { return v; } \
    T operator+(const T& other) const { return v + other; } \
    T operator-(const T& other) const { return v - other; } \
    T operator*(const T& other) const { return v * other; } \
    T operator/(const T& other) const { return v / other; } \
    T operator%(const T& other) const { return v % other; } \
    bool operator<(const T& other) const { return v < other; } \
    bool operator>(const T& other) const { return v > other; } \
    bool operator<=(const T& other) const { return v <= other; } \
    bool operator>=(const T& other) const { return v >= other; } \
    bool operator==(const T& other) const { return v == other; } \
    bool operator!=(const T& other) const { return v != other; }

