#pragma once
#include <string>

struct Token {
    std::string lexeme;   // 词素，比如 "int"、"123"、"=="
    std::string type;     // 类型串，比如 "INTSYM"、"INT"、"RO"
    // 实验二再来加 enum kind、行号列号等
};