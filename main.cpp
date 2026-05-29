#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <iterator>

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string src{std::istreambuf_iterator<char>(std::cin),
                    std::istreambuf_iterator<char>()};

    auto lex = tokenize(src);
    if (!lex.error.empty()) { std::cout << "Syntax Error"; return 0; }

    auto par = parse(lex.tokens);
    if (!par.ok) { std::cout << "Syntax Error"; return 0; }   // 无换行

    // 符号表
    std::cout << par.symbols.size() << '\n';
    for (const auto& sym : par.symbols)
        std::cout << sym.name << ' ' << sym.type << " null " << sym.offset << '\n';
    // 临时变量数
    std::cout << par.temp_count << '\n';
    // 四元式
    std::cout << par.quads.size() << '\n';
    for (size_t i = 0; i < par.quads.size(); ++i) {
        const auto& q = par.quads[i];
        std::cout << i << ": (" << q.op << ',' << q.arg1 << ','
                    << q.arg2 << ',' << q.result << ")\n";
    }
    return 0;
}