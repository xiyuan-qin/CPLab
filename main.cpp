#include "lexer.h"
#include <iostream>
#include <iterator>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string src{std::istreambuf_iterator<char>(std::cin),
                    std::istreambuf_iterator<char>()}; // istream迭代器绑定到 cin 开头，到istream结尾 一起赋值给 src

    auto res = tokenize(src);

    if (!res.error.empty()) {
        std::cout << res.error << '\n';
        return 0;
    }
    for (const auto& t : res.tokens) {
        std::cout << t.lexeme << ' ' << t.type << '\n';
    }
    return 0;
}