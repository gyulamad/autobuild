#pragma once

#include <vector>
#include <functional>
#include <thread>

using namespace std;

// Parallel each function using manual threading

template <typename VectorType>
void peach(VectorType& things, function<void(typename conditional<is_const<VectorType>::value, const typename VectorType::value_type&, typename VectorType::value_type&>::type, size_t)> callback) {
    vector<thread> threads;
    threads.reserve(things.size());

    for (size_t i = 0; i < things.size(); ++i)
        threads.emplace_back([i, &things, &callback]() {
            callback(things[i], i);
        });

    for (auto& t : threads)
        if (t.joinable()) t.join();
}
