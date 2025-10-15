#pragma once

#include <iostream>

using namespace std;

void header_only() {
    cout << "header_only v1" << endl;
}

#ifdef TEST

TEST(test_testing1) {
    int a = 1;
    int b = a + 1;
    assert(b == 2);
}

#endif

