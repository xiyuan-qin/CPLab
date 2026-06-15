#include "ir.h"
#include "parse_input.h"
#include "codegen.h"
#include <iostream>

int main() {
    std::ios::sync_with_stdio(false);
    State st;
    input(st, std::cin);
    run(st);
    return 0;
}
