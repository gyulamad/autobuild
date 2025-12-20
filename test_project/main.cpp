// _DEPENDENCY: testcreator/testlib:v1.2.3
// _DEPENDENCY: creator2/lib2, lib3:v0.0.3, somelib4
// DEPENDENCY: fltk
#include "../../../cpptools/misc/TEST.hpp"
#include "header_only.hpp"
#include "with_cpp.h"

int main() {
    tester.run({});
    header_only();
    with_cpp();
    return 0;
}