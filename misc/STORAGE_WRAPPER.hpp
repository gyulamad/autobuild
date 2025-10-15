#pragma once

#define STORAGE_WRAPPER(ST, T, v) \
    const ST<T>::iterator begin() { return v.begin(); } \
    const ST<T>::iterator end() { return v.end(); } \
    const ST<T>::const_iterator begin() const { return v.begin(); } \
    const ST<T>::const_iterator end() const { return v.end(); } \
    void push_back(T value) { v.push_back(value); } \
    size_t size() const { return v.size(); } \
    void reserve(size_t n) { v.reserve(n); } \
    bool empty() const { return v.empty(); } \
    const T& operator[](size_t at) const { return v[at]; } \
    T& operator[](size_t at) { return v[at]; } \
    ST<T>& operator=(const ST<T>& v) { this->v = v; return this->v; } \
    void resize(size_t size) { v.resize(size); } \
    void resize(size_t size, T& x) { v.resize(size, x); } \
    void clear() { v.clear(); }

