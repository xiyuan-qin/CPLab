#pragma once
#include "token.h"
#include <vector>
#include <string>

struct ParseResult {
    bool ok;
    std::string error;     // ok=false 时为 "Syntax Error"
    // todo : 后续 D/E/F 阶段加：符号表、四元式
};

ParseResult parse(const std::vector<Token>& tokens);