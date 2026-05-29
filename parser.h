#pragma once
#include "token.h"
#include "semantic.h"
#include <vector>
#include <string>

struct ParseResult {
    bool ok;
    std::string error;
    std::vector<SymEntry> symbols;
    std::vector<Quad> quads;
    int temp_count = 0;
};

ParseResult parse(const std::vector<Token>& tokens);