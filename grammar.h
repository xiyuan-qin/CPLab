#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace grammar {

// 所有符号统一用 int 表示。约定：
//   终结符：[0, NUM_TERMS)
//   非终结符：[NUM_TERMS, NUM_TERMS + NUM_NONTERMS)
//   特殊：END_SYM = 终结符 #（输入结束）
using Sym = int;

// 终结符（顺序要和 TokenKind 对得上，否则要写映射）
enum Terminal : Sym {
    T_UINT = 0, T_UFLOAT, T_ID,
    T_INT, T_DOUBLE, T_SCANF, T_PRINTF, T_IF, T_THEN, T_WHILE, T_DO,
    T_ASSIGN,
    T_EQ, T_NEQ, T_LT, T_LE, T_GT, T_GE,
    T_PLUS, T_MINUS, T_TIMES, T_DIVIDE,
    T_AND, T_OR, T_NOT,
    T_COMMA, T_SEMI,
    T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE,
    T_END,        // 输入末尾的 #
    NUM_TERMS
};

// 非终结符
enum NonTerminal : Sym {
    N_PROG = NUM_TERMS,
    N_SUBPROG, N_M, N_N,
    N_VARIABLES, N_VARIABLE, N_T, N_ID,
    N_STATEMENT, N_ASSIGN, N_SCANF, N_SCANF_BEGIN,
    N_PRINTF, N_PRINTF_BEGIN, N_L,
    N_EXPR, N_ORITEM, N_ANDITEM, N_NOITEM,
    N_RELITEM, N_ITEM, N_FACTOR,
    N_B, N_BORTERM, N_BANDTERM, N_BFACTOR,
    N_PLUS_MINUS, N_MUL_DIV, N_REL,
    NUM_SYMBOLS
};
constexpr int NUM_NONTERMS = NUM_SYMBOLS - NUM_TERMS;

inline bool is_terminal(Sym s)    { return s < NUM_TERMS; }
inline bool is_nonterminal(Sym s) { return s >= NUM_TERMS && s < NUM_SYMBOLS; }

// 产生式：lhs -> rhs[0] rhs[1] ...
// 空产生式 rhs 为空
struct Production {
    Sym lhs;
    std::vector<Sym> rhs;
    int id;          // 产生式编号，归约时按这个分发语义动作
};

// 文法对象（单例）
struct Grammar {
    std::vector<Production> prods;        // 产生式列表，prods[0] 是增广产生式 S' -> PROG
    std::vector<std::vector<int>> prods_of;   // prods_of[A] = 以 A 为左部的产生式编号列表
    std::vector<std::unordered_set<Sym>> first;  // first[S] = FIRST(S)
    Sym start_symbol;     // PROG
    Sym aug_start;        // S'
    int aug_prod_id = 0;  // S' -> PROG 的编号

    static const Grammar& instance();

    // 计算字符串 α 的 FIRST 集（α 可能含多个符号）
    std::unordered_set<Sym> first_of(const std::vector<Sym>& alpha,
                                    int start = 0) const;

    // 调试用
    std::string sym_name(Sym s) const;
    std::string prod_to_string(int id) const;
};

} // namespace grammar