#include "ir.h"
#include <cctype>

bool Program::is_temp(const std::string& s) const {
    // 临时变量形如 T0_i / T12_d
    return s.size() >= 2 && s[0] == 'T' && std::isdigit((unsigned char)s[1]);
}

bool Program::is_var(const std::string& s) const {
    if (s.empty() || s == "-") return false;
    // 变量：TBn（符号表变量）或 Tn_x（临时变量）
    // 字面量：纯数字 / 浮点 / 负数
    if (s[0] == 'T') return true;          // TB.. 或 T.._.
    return false;                          // 其余视为字面量/常数
}

Symbol* Program::find(const std::string& name) {
    auto it = sym_index.find(name);
    return it == sym_index.end() ? nullptr : &symbols[it->second];
}