#pragma once
#include <string>
#include <vector>
#include <unordered_map>

constexpr int TYPE_INT = 0;
constexpr int TYPE_DOUBLE = 1;

struct SymEntry {
    std::string name;
    int type;          // 0=int, 1=double
    int offset;
    bool assigned = false;   // 阶段 F 用：是否被赋过值
};

struct Quad {
    std::string op, arg1, arg2, result;
};

// 文法符号的综合/继承属性（统一结构，不同符号用不同字段）
struct Attr {
    std::string lexeme;    // 终结符词素：id 名 / UINT / UFLOAT 字面量
    std::string name;      // ID.name
    std::string place;     // 表达式结果位置（TBn 或 Tn_x）
    int type = TYPE_INT;
    int width = 0;
    std::string op;        // 运算符
    int quad = 0;          // N.quad
    std::vector<int> nextlist, truelist, falselist;
};

// 语义上下文：贯穿整个分析过程
class Semantic {
public:
    std::vector<SymEntry> symbols;
    std::unordered_map<std::string, int> sym_index;
    std::vector<Quad> quads;
    int temp_count = 0;
    int OFFSET = 0;
    bool error = false;

    bool enter(const std::string& name, int type, int offset);   // false = 重复定义
    std::string lookup(const std::string& name, bool check_assigned);  // "" = 出错
    int lookup_type(const std::string& name);
    void mark_assigned(const std::string& name);

    std::string newtemp(int type);
    int nextquad() const { return static_cast<int>(quads.size()); }
    void gen(const std::string& op, const std::string& a,
            const std::string& b, const std::string& r);
    void backpatch(const std::vector<int>& list, int target);
};

std::vector<int> mklist(int q);
std::vector<int> mklist();
std::vector<int> merge(const std::vector<int>& a, const std::vector<int>& b);