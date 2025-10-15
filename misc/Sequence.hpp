#pragma once

/** Usage Example: *************************************



****************************************************** */

#include <vector>
#include <unordered_set>
#include "array_unique.hpp"
#include "array_slice.hpp"
#include "Logger.hpp"

using namespace std;

template<typename T>
class Sequence: public vector<T> {
public:
    using vector<T>::vector;
    
    // Move constructor
    Sequence(Sequence&& other) noexcept 
        : vector<T>(std::move(other)), 
          mode_ordered(other.mode_ordered),
          orders(std::move(other.orders)) {}
    
    // Move assignment
    Sequence& operator=(Sequence&& other) noexcept {
        vector<T>::operator=(std::move(other));
        mode_ordered = other.mode_ordered;
        orders = std::move(other.orders);
        return *this;
    }

    // Copy constructor
    Sequence(const Sequence& other) 
        : vector<T>(other),
          mode_ordered(other.mode_ordered),
          orders(other.orders) {}

    // Copy assignment
    Sequence& operator=(const Sequence& other) {
        vector<T>::operator=(other);
        mode_ordered = other.mode_ordered;
        orders = other.orders;
        return *this;
    }

    // Keep your existing move operations
    // Sequence(Sequence&& other) noexcept = default;
    // Sequence& operator=(Sequence&& other) noexcept = default;

    virtual ~Sequence() = default;

    void reconstruct(vector<T> elements, const vector<size_t>& orders = {}, bool strict = false) {
        this->orders = array_unique(orders);
        if (strict && elements.size() != this->orders.size())
            throw ERROR("Elements and orders size mismatch");
        this->clear();
        for (const T& element: elements) this->push_back(element);
        fix_orders();
    }

    void set_mode_ordered(bool mode_ordered) { this->mode_ordered = mode_ordered; }

    const vector<size_t>& get_orders_cref() { 
        fix_orders();
        return orders;
    }

    // vector<T>& get_elements_cref() { return *this; }

    void insert_at(size_t at, const T& element) {
        fix_orders();
        this->push_back(element);
        size_t last = this->size() - 1;
        orders.push_back(last);
        replace_with(last, at);
    }

    void move_to(size_t at, size_t to) {
        if (at < to) move_backward(at, to - at);
        if (at > to) move_forward(at, at - to);
    }

    void replace_with(size_t at, size_t to) {
        fix_orders();
        if (at >= orders.size() || to >= orders.size())
            throw ERROR("Order position is out of bounds or invalid");
        if (at == to) return;
        size_t tmp = orders[at]; // TODO: is there a way to swap two elements in an unordered set??
        orders[at] = orders[to];
        orders[to] = tmp;
    }

    void move_backward(size_t at, size_t d = 1) {
        fix_orders();
        if (at + 1 >= orders.size()) return;
        for (size_t i = 0; i < d; i++)
            replace_with(at + i, at + i + 1);
    }

    void move_forward(size_t at, size_t d = 1) {
        if (at == 0) return;
        for (size_t i = 0; i < d; i++)
            replace_with(at - i, at - i - 1);
    }

    //=== Mode-Specific Accessors ===//
    T& operator[](size_t at) {
        fix_orders();
        return vector<T>::operator[](mode_ordered ? orders[at] : at);
    }

    const T& operator[](size_t at) const {
        return vector<T>::operator[](mode_ordered ? orders[at] : at);
    }

    T& at(size_t at) {
        fix_orders();
        return vector<T>::at(mode_ordered ? orders[at] : at);
    }

    const T& at(size_t at) const {
        return vector<T>::at(mode_ordered ? orders[at] : at);
    }

protected:
    bool mode_ordered = false;
    vector<size_t> orders = {};

    void fix_orders() {
        if (orders.size() > this->size()) orders = array_slice(orders, 0, this->size());
        for (size_t n = orders.size(); n < this->size(); n++) orders.push_back(n);
    }
};


#ifdef TEST

#include <string>
#include "str_contains.hpp"

TEST(test_Sequence_basic_operations) {
    Sequence<string> seq;
    assert(seq.empty() && "Sequence should be empty initially");
    
    seq.push_back("First");
    seq.push_back("Second");
    assert(seq.size() == 2 && "Sequence should have 2 elements");
    assert(seq[0] == "First" && "First element should be 'First'");
    assert(seq[1] == "Second" && "Second element should be 'Second'");
}

TEST(test_Sequence_insert_operations) {
    Sequence<string> seq;
    seq.push_back("A");
    seq.push_back("C");
    
    seq.insert_at(1, "B");  // Insert at position 1
    assert(seq.size() == 3 && "Sequence should have 3 elements");
    seq.set_mode_ordered(true);
    assert(seq[0] == "A" && "First element should be 'A'");
    assert(seq[1] == "B" && "Second element should be 'B'");
    assert(seq[2] == "C" && "Third element should be 'C'");
    
    seq.insert_at(0, "Start");  // Insert at beginning
    assert(seq[0] == "Start" && "New first element should be 'Start'");
}

TEST(test_Sequence_move_operations) {
    Sequence<std::string> seq;
    seq.push_back("A");
    seq.push_back("B");
    seq.push_back("C");
    
    // Initial order: A(0), B(1), C(2)
    
    seq.move_backward(0);  // Move A down (swap with B)
    // Expected order after move: B(1), A(0), C(2)
    seq.set_mode_ordered(true);
    assert(seq[0] == "B" && "First element should now be 'B'");
    assert(seq[1] == "A" && "Second element should now be 'A'");
    assert(seq[2] == "C" && "Third element should remain 'C'");
    
    seq.move_forward(2);  // Move C up (swap with A)
    // Expected order after move: B(1), C(2), A(0)
    assert(seq[0] == "B" && "First element should remain 'B'");
    assert(seq[1] == "C" && "Second element should now be 'C'");
    assert(seq[2] == "A" && "Third element should now be 'A'");
}

TEST(test_Sequence_iteration) {
    Sequence<string> seq;
    seq.push_back("X");
    seq.push_back("Y");
    seq.push_back("Z");
    
    string concatenated;
    for (const auto& item : seq)
        concatenated += item;
        
    assert(concatenated == "XYZ" && "Iteration should produce 'XYZ'");
}

TEST(test_Sequence_at_out_of_range) {
    Sequence<int> seq;
    seq.push_back(1);
    seq.push_back(2);
    
    bool threw = false;
    try {
        seq.at(2);  // Should throw
    } catch (exception& e) {
        string what = e.what();
        assert(str_contains(what, "2"));
        threw = true;
    }
    assert(threw && "at() should throw for out-of-range access");
}

TEST(test_Sequence_clear_operation) {
    Sequence<double> seq;
    seq.push_back(1.1);
    seq.push_back(2.2);
    
    seq.clear();
    assert(seq.empty() && "Sequence should be empty after clear");
    assert(seq.size() == 0 && "Size should be 0 after clear");
    
    // Verify we can add elements after clear
    seq.push_back(3.3);
    assert(seq.size() == 1 && "Should be able to add after clear");
}

TEST(test_Sequence_move_semantics) {
    Sequence<string> seq;
    string value = "Movable";
    
    seq.push_back(std::move(value));  // Test rvalue insert
    assert(value.empty() && "Original string should be empty after move");
    assert(seq[0] == "Movable" && "Moved string should be in sequence");
    
    Sequence<string> seq2;
    seq2.insert_at(0, string("Temporary"));  // Test rvalue positional insert
    assert(seq2[0] == "Temporary" && "Temporary should be in sequence");
}

TEST(test_Sequence_large_sequence) {
    Sequence<int> seq;
    const int count = 1000;
    
    for (int i = 0; i < count; i++) seq.push_back(i);
    
    assert(seq.size() == count && "Should have 1000 elements");
    seq.set_mode_ordered(true);
    for (int i = 0; i < count; i++)
        assert(seq[i] == i && "Elements should match their values");
    
    // Test moving elements in large sequence
    int original_500 = seq[500];
    int original_501 = seq[501];
    
    seq.move_backward(500);  // Move element 500 down (swap with 501)
    
    assert(seq[500] == original_501 && "Element 500 should now be the original 501");
    assert(seq[501] == original_500 && "Element 501 should now be the original 500");
    
    // Verify other elements remain unchanged
    for (int i = 0; i < count; i++)
        if (i != 500 && i != 501)
            assert(seq[i] == i && "Other elements should remain unchanged");
}

TEST(test_Sequence_initializer_list_constructor) {
    Sequence<int> seq{1, 2, 3};
    assert(seq.size() == 3);
    assert(seq[0] == 1 && seq[1] == 2 && seq[2] == 3);
}

TEST(test_Sequence_size_constructor) {
    Sequence<string> seq(3);
    assert(seq.size() == 3);
    assert(seq[0].empty() && seq[1].empty() && seq[2].empty());
}

TEST(test_Sequence_size_value_constructor) {
    Sequence<int> seq(4, 42);
    assert(seq.size() == 4);
    for (size_t i = 0; i < 4; i++)
        assert(seq[i] == 42);
}

TEST(test_Sequence_resize) {
    Sequence<int> seq{1, 2, 3};
    seq.resize(5);
    assert(seq.size() == 5);
    assert(seq[3] == 0 && seq[4] == 0);  // Default int is 0
    
    seq.resize(2);
    assert(seq.size() == 2);
    assert(seq[0] == 1 && seq[1] == 2);
    
    seq.resize(4, 99);
    assert(seq.size() == 4);
    assert(seq[2] == 99 && seq[3] == 99);
}

TEST(test_Sequence_capacity) {
    Sequence<int> seq;
    assert(seq.capacity() == 0);
    
    seq.reserve(100);
    assert(seq.capacity() >= 100);
    assert(seq.empty());
    
    // Add elements to test growth
    for (int i = 0; i < 200; i++) seq.push_back(i);
    assert(seq.size() == 200);
}

TEST(test_Sequence_edge_cases) {
    Sequence<int> seq{1}; // Initial: [1]
    
    // Move single element (should have no effect)
    seq.move_forward(0); // Still [1]
    seq.move_backward(0); // Still [1]
    assert(seq[0] == 1 && seq.size() == 1); // TODO: ASSERT FAILS, because storage: [1, 0]; orders: [0, 1]
    
    // Insert at boundaries
    seq.insert_at(0, 0); // Becomes [0, 1]
    seq.set_mode_ordered(true); // using ordered access to read back the results
    assert(seq[0] == 0 && seq[1] == 1);
    
    seq.insert_at(seq.size(), 2); // Becomes [0, 1, 2]
    assert(seq[0] == 0 && seq[1] == 1 && seq[2] == 2);
    
    // Empty sequence operations
    Sequence<int> empty;
    bool threw = false;
    try {
        int i = empty.at(0); // Should throw
        assert(i == 0); // just to make i not unused
    } catch (const exception& e) {
        string what = e.what();
        assert(str_contains(what, "0"));
        threw = true;
    }
    assert(threw);
}

TEST(test_Sequence_order_management) {
    Sequence<string> seq{"A", "B", "C"};
    auto orders = seq.get_orders_cref();
    assert(orders.size() == 3);
    assert(orders[0] == 0 && orders[1] == 1 && orders[2] == 2);
    
    // Test reconstruct
    vector<string> elements{"X", "Y", "Z"};
    vector<size_t> new_order{2, 0, 1};
    seq.reconstruct(elements, new_order);
    seq.set_mode_ordered(true);
    assert(seq[0] == "Z" && seq[1] == "X" && seq[2] == "Y");
    
    // Test invalid reconstruct
    bool threw = false;
    try {
        seq.reconstruct({"A"}, {0, 1}, true);  // Size mismatch
    } catch (exception& e) {
        string what = e.what();
        assert(str_contains(what, "Elements and orders size mismatch"));
        threw = true;
    }
    assert(threw);
}

TEST(test_Sequence_const_correctness) {
    const Sequence<int> seq{1, 2, 3};
    
    assert(seq[0] == 1);
    assert(seq.at(1) == 2);
    assert(!seq.empty());
    
    int sum = 0;
    for (const auto& item : seq)
        sum += item;
    assert(sum == 6);
}

TEST(test_Sequence_exceptions) {
    Sequence<int> seq;
    
    // Test at() out of range
    bool threw = false;
    try {
        seq.at(0);
    } catch (exception& e) {
        string what = e.what();
        assert(str_contains(what, "0"));
        threw = true;
    }
    assert(threw);
    
    // Test invalid reconstruct
    seq.push_back(1);
    seq.push_back(2);
    threw = false;
    try {
        seq.reconstruct({1, 2}, {0, 0}, true);  // Duplicate indices
    } catch (exception& e) {
        string what = e.what();
        assert(str_contains(what, "Elements and orders size mismatch"));
        threw = true;
    }
    assert(threw);
}

TEST(test_Sequence_move_operations2) {
    Sequence<string> seq1;
    seq1.push_back("A");
    seq1.push_back("B");
    
    // Move constructor
    Sequence<string> seq2(std::move(seq1));
    assert(seq1.empty());
    assert(seq2.size() == 2);
    
    // Move assignment
    seq1 = std::move(seq2);
    assert(seq2.empty());
    assert(seq1.size() == 2);
}

TEST(test_Sequence_large_scale) {
    const size_t N = 100000;
    Sequence<int> seq;
    
    // Test insertion performance
    for (size_t i = 0; i < N; i++)
        seq.push_back(static_cast<int>(i));
    
    // Test access performance
    seq.set_mode_ordered(true);
    for (size_t i = 0; i < N; i += N/10)
        assert(seq[i] == static_cast<int>(i));
    
    // Test move operations
    for (size_t i = 0; i < N; i += N/10)
        seq.move_backward(i);
}

TEST(test_Sequence_template_types) {
    // Test with POD type
    Sequence<float> floats;
    floats.push_back(1.1f);
    floats.push_back(2.2f);
    
    // Test with objects
    Sequence<vector<int>> vectors;
    vectors.push_back(vector<int>{1, 2});
    vectors.push_back(vector<int>{3, 4});
    
    // Test with move-only type
    Sequence<unique_ptr<int>> ptrs;
    ptrs.push_back(make_unique<int>(42));
    assert(*ptrs[0] == 42);
}

#endif
