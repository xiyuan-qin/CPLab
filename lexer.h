#pragma once
#include "token.h"
#include <string>
#include <vector>

class LexResult {
public:
    std::vector<Token> tokens;
    std::string error;    // 非空 = 有错；只保留第一个错
};

LexResult tokenize(const std::string& src);// 返回一个lex Result，里面既含处理完的token，也含对应的错误信息（只能同时存在一个）