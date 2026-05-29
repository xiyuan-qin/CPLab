#pragma once
#include <string>

// 词法 + 语法共享的 token 类型枚举
// 要和 grammar.h 里的终结符 enum 对得上
enum class TokenKind {
    // 字面量
    UINT, UFLOAT, ID,
    // 关键字
    INT, DOUBLE, SCANF, PRINTF, IF, THEN, WHILE, DO,
    // 运算符
    ASSIGN_OP,    // =
    EQ, NEQ, LT, LE, GT, GE,   // == != < <= > >=
    PLUS, MINUS, TIMES, DIVIDE,
    AND, OR, NOT,              // && || !
    // 界符
    COMMA, SEMICOLON,
    LPAREN, RPAREN, LBRACE, RBRACE,
    // 文件结尾（语法分析用）
    END
};

struct Token {
    std::string lexeme;
    std::string type;     // 一：仍保留字符串类型
    TokenKind   kind;     // 二：枚举形式
};