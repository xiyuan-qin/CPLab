#include "lexer.h"
#include <iostream>
#include <iterator>
#include <format>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string src{std::istreambuf_iterator<char>(std::cin),
                    std::istreambuf_iterator<char>()};

    auto res = tokenize(src);

    if (!res.error.empty()) {
        std::cout << res.error << '\n';
        return 0;
    }
    for (const auto& [lex, ty] : res.tokens) {
        std::cout << std::format("{} {}\n", lex, ty);
    }
    return 0;
}