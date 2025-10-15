#pragma once

using namespace std;

// Bitmask class to manage configuration flags
class Bitmask {
private:
    int mask; // Stores the current state of all flags

public:
    // Constructor: Initialize with a starting value (default 0)
    Bitmask(int initial = 0) : mask(initial) {}

    // Set one or multiple flags using bitwise OR
    void set(int flags) {
        mask |= flags;
    }

    // Clear one or multiple flags using bitwise AND with NOT
    void clear(int flags) {
        mask &= ~flags;
    }

    // Check if one or more flags are set using bitwise AND
    bool get(int flags) const {
        return (mask & flags) == flags;
    }

    // Get the raw mask value
    int value() const {
        return mask;
    }
};

#ifdef TEST

// Test cases using TEST macro
TEST(test_Bitmask_initial_state) {
    Bitmask bm;
    assert(bm.value() == 0 && "Initial mask should be 0");
    assert(!bm.get(1) && "No flags should be set initially");
}

TEST(test_Bitmask_set_single_flag) {
    Bitmask bm;
    const int FLAG = 1 << 0; // 0b0001
    bm.set(FLAG);
    assert(bm.get(FLAG) && "FLAG should be set");
    assert(bm.value() == FLAG && "Mask value should match FLAG");
}

TEST(test_Bitmask_set_multiple_flags) {
    Bitmask bm;
    const int THIS = 1 << 0; // 0b0001
    const int THAT = 1 << 1; // 0b0010
    bm.set(THIS | THAT);
    assert(bm.get(THIS) && "THIS should be set");
    assert(bm.get(THAT) && "THAT should be set");
    assert(bm.get(THIS | THAT) && "Both THIS and THAT should be set");
    assert(bm.value() == (THIS | THAT) && "Mask value should match combined flags");
}

TEST(test_Bitmask_clear_single_flag) {
    Bitmask bm;
    const int THIS = 1 << 0; // 0b0001
    const int THAT = 1 << 1; // 0b0010
    bm.set(THIS | THAT);
    bm.clear(THIS);
    assert(!bm.get(THIS) && "THIS should be cleared");
    assert(bm.get(THAT) && "THAT should remain set");
    assert(bm.value() == THAT && "Mask value should only have THAT");
}

TEST(test_Bitmask_clear_multiple_flags) {
    Bitmask bm;
    const int THIS = 1 << 0; // 0b0001
    const int THAT = 1 << 1; // 0b0010
    const int FOO = 1 << 2; // 0b0100
    bm.set(THIS | THAT | FOO);
    bm.clear(THIS | THAT);
    assert(!bm.get(THIS) && "THIS should be cleared");
    assert(!bm.get(THAT) && "THAT should be cleared");
    assert(bm.get(FOO) && "FOO should remain set");
    assert(bm.value() == FOO && "Mask value should only have FOO");
}

TEST(test_Bitmask_get_unset_flag) {
    Bitmask bm;
    const int THIS = 1 << 0; // 0b0001
    const int FOO = 1 << 2; // 0b0100
    bm.set(THIS);
    assert(!bm.get(FOO) && "FOO should not be set");
}

TEST(test_Bitmask_initial_nonzero) {
    const int THIS = 1 << 0; // 0b0001
    const int THAT = 1 << 1; // 0b0010
    Bitmask bm(THIS | THAT);
    assert(bm.get(THIS) && "THIS should be set from constructor");
    assert(bm.get(THAT) && "THAT should be set from constructor");
    assert(bm.value() == (THIS | THAT) && "Mask value should match initial value");
}

#endif