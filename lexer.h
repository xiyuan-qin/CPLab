#pragma once
#include "token.h"
#include <string>
#include <vector>

struct LexResult {
    std::vector<Token> tokens;
    std::string error;    // 非空 = 有错；只保留第一个错
};

LexResult tokenize(const std::string& src);