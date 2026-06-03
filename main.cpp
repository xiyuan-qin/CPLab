#include "ir.h"
#include "parse_input.h"
#include "blocks.h"
#include "liveness.h"
#include "codegen.h"
#include <iostream>
#include <iterator>

int main() {
    std::ios::sync_with_stdio(false);
    std::string src{std::istreambuf_iterator<char>(std::cin),
                    std::istreambuf_iterator<char>()};
    Program prog = parse_input(src);
    auto blocks = partition_blocks(prog);
    compute_liveness(prog, blocks);
    auto code = generate_code(prog, blocks);
    for (auto& line : code) std::cout << line << '\n';
    return 0;
}