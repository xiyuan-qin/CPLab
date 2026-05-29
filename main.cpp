#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <iterator>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string src{std::istreambuf_iterator<char>(std::cin),
                    std::istreambuf_iterator<char>()};

    auto lex_res = tokenize(src);
    if (!lex_res.error.empty()) {
        // 实验二保证无词法错，但保险起见
        std::cout << "Syntax Error";
        return 0;
    }

    auto par_res = parse(lex_res.tokens);
    if (!par_res.ok) {
        std::cout << "Syntax Error";   // 注意无换行
        return 0;
    }

    // 阶段 C：成功就先打印一行占位
    std::cout << "ACC\n";
    return 0;
}