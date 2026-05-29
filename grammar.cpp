#include "grammar.h"
#include <algorithm>

namespace grammar {

namespace {

// 把产生式紧凑表达：{lhs, {rhs...}}
const std::vector<std::pair<Sym, std::vector<Sym>>> RAW_PRODS = {
    // 0 号留给增广产生式 S' -> PROG，在 init 时注入

    // 程序结构
    {N_PROG,        {N_SUBPROG}},
    {N_SUBPROG,     {N_M, N_VARIABLES, N_STATEMENT}},
    {N_M,           {}},                          // ε
    {N_N,           {}},                          // ε

    // 变量声明
    {N_VARIABLES,   {N_VARIABLES, N_VARIABLE, T_SEMI}},
    {N_VARIABLES,   {N_VARIABLE, T_SEMI}},
    {N_T,           {T_INT}},
    {N_T,           {T_DOUBLE}},
    {N_ID,          {T_ID}},
    {N_VARIABLE,    {N_T, N_ID}},
    {N_VARIABLE,    {N_VARIABLE, T_COMMA, N_ID}},

    // 语句
    {N_STATEMENT,   {N_ASSIGN}},
    {N_STATEMENT,   {N_SCANF}},
    {N_STATEMENT,   {N_PRINTF}},
    {N_STATEMENT,   {}},                          // ε
    {N_STATEMENT,   {T_LBRACE, N_L, T_SEMI, T_RBRACE}},
    {N_STATEMENT,   {T_WHILE, N_N, N_B, T_DO, N_N, N_STATEMENT}},
    {N_STATEMENT,   {T_IF, N_B, T_THEN, N_N, N_STATEMENT}},
    {N_ASSIGN,      {N_ID, T_ASSIGN, N_EXPR}},
    {N_L,           {N_L, T_SEMI, N_N, N_STATEMENT}},
    {N_L,           {N_STATEMENT}},

    // 数值表达式
    {N_EXPR,        {N_EXPR, T_OR, N_ORITEM}},
    {N_EXPR,        {N_ORITEM}},
    {N_ORITEM,      {N_ORITEM, T_AND, N_ANDITEM}},
    {N_ORITEM,      {N_ANDITEM}},
    {N_ANDITEM,     {N_NOITEM}},
    {N_ANDITEM,     {T_NOT, N_NOITEM}},
    {N_NOITEM,      {N_NOITEM, N_REL, N_RELITEM}},
    {N_NOITEM,      {N_RELITEM}},
    {N_RELITEM,     {N_RELITEM, N_PLUS_MINUS, N_ITEM}},
    {N_RELITEM,     {N_ITEM}},
    {N_ITEM,        {N_FACTOR}},
    {N_ITEM,        {N_ITEM, N_MUL_DIV, N_FACTOR}},
    {N_FACTOR,      {N_ID}},
    {N_FACTOR,      {T_UINT}},
    {N_FACTOR,      {T_UFLOAT}},
    {N_FACTOR,      {T_LPAREN, N_EXPR, T_RPAREN}},
    {N_FACTOR,      {N_PLUS_MINUS, N_FACTOR}},

    // 条件表达式（布尔，要做回填）
    {N_B,           {N_B, T_OR, N_N, N_BORTERM}},
    {N_B,           {N_BORTERM}},
    {N_BORTERM,     {N_BORTERM, T_AND, N_N, N_BANDTERM}},
    {N_BORTERM,     {N_BANDTERM}},
    {N_BANDTERM,    {T_LPAREN, N_B, T_RPAREN}},
    {N_BANDTERM,    {T_NOT, N_BANDTERM}},
    {N_BANDTERM,    {N_BFACTOR, N_REL, N_BFACTOR}},
    {N_BANDTERM,    {N_BFACTOR}},
    {N_BFACTOR,     {T_UINT}},
    {N_BFACTOR,     {T_UFLOAT}},
    {N_BFACTOR,     {N_ID}},

    // 运算符
    {N_PLUS_MINUS,  {T_PLUS}},
    {N_PLUS_MINUS,  {T_MINUS}},
    {N_MUL_DIV,     {T_TIMES}},
    {N_MUL_DIV,     {T_DIVIDE}},
    {N_REL,         {T_EQ}},
    {N_REL,         {T_NEQ}},
    {N_REL,         {T_LT}},
    {N_REL,         {T_LE}},
    {N_REL,         {T_GT}},
    {N_REL,         {T_GE}},

    // 读写
    {N_SCANF,         {N_SCANF_BEGIN, T_RPAREN}},
    {N_SCANF_BEGIN,   {N_SCANF_BEGIN, T_COMMA, N_ID}},
    {N_SCANF_BEGIN,   {T_SCANF, T_LPAREN, N_ID}},
    {N_PRINTF,        {N_PRINTF_BEGIN, T_RPAREN}},
    {N_PRINTF_BEGIN,  {T_PRINTF, T_LPAREN, N_ID}},
    {N_PRINTF_BEGIN,  {N_PRINTF_BEGIN, T_COMMA, N_ID}},
};

Grammar build_grammar() {
    Grammar g;
    g.start_symbol = N_PROG;
    g.aug_start    = NUM_SYMBOLS;   // S' 用一个新编号，超出 NUM_SYMBOLS

    // 0 号产生式：S' -> PROG
    g.prods.push_back({g.aug_start, {N_PROG}, 0});

    for (size_t i = 0; i < RAW_PRODS.size(); ++i) {
        g.prods.push_back({RAW_PRODS[i].first, RAW_PRODS[i].second,
                           static_cast<int>(i + 1)});
    }

    // 反向索引：每个非终结符的产生式列表
    g.prods_of.resize(NUM_SYMBOLS + 1);
    for (const auto& p : g.prods) g.prods_of[p.lhs].push_back(p.id);

    // 计算 FIRST 集（不动点迭代）
    g.first.resize(NUM_SYMBOLS + 1);
    for (Sym t = 0; t < NUM_TERMS; ++t) g.first[t].insert(t);  // FIRST(终结符) = {自己}

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& p : g.prods) {
            auto& fa = g.first[p.lhs];
            size_t before = fa.size();

            if (p.rhs.empty()) {
                fa.insert(T_END);    // 用 T_END 表示 ε，归约时再区分
                // 实际上 ε 用一个专用 sentinel 更干净，这里偷个懒
            } else {
                bool all_nullable = true;
                for (Sym s : p.rhs) {
                    for (Sym f : g.first[s]) {
                        if (f != T_END) fa.insert(f);
                    }
                    if (g.first[s].count(T_END) == 0) {
                        all_nullable = false;
                        break;
                    }
                }
                if (all_nullable) fa.insert(T_END);
            }
            if (fa.size() != before) changed = true;
        }
    }
    return g;
}

} // namespace

const Grammar& Grammar::instance() {
    static Grammar g = build_grammar();
    return g;
}

std::unordered_set<Sym> Grammar::first_of(const std::vector<Sym>& alpha, int start) const {
    std::unordered_set<Sym> result;
    bool all_nullable = true;
    for (size_t i = start; i < alpha.size(); ++i) {
        Sym s = alpha[i];
        for (Sym f : first[s]) if (f != T_END) result.insert(f);
        if (first[s].count(T_END) == 0) { all_nullable = false; break; }
    }
    if (all_nullable) result.insert(T_END);
    return result;
}

std::string Grammar::sym_name(Sym s) const {
    static const char* TERM_NAMES[] = {
        "UINT","UFLOAT","id","int","double","scanf","printf","if","then","while","do",
        "=","==","!=","<","<=",">",">=","+","-","*","/","&&","||","!",",",";",
        "(",")","{","}","#"
    };
    static const char* NT_NAMES[] = {
        "PROG","SUBPROG","M","N","VARIABLES","VARIABLE","T","ID",
        "STATEMENT","ASSIGN","SCANF","SCANF_BEGIN","PRINTF","PRINTF_BEGIN","L",
        "EXPR","ORITEM","ANDITEM","NOITEM","RELITEM","ITEM","FACTOR",
        "B","BORTERM","BANDTERM","BFACTOR",
        "PLUS_MINUS","MUL_DIV","REL"
    };
    if (s < NUM_TERMS) return TERM_NAMES[s];
    if (s == aug_start) return "S'";
    return NT_NAMES[s - NUM_TERMS];
}

} // namespace grammar