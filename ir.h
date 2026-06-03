#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// 一个操作数的待用/活跃信息
struct UseInfo {
    int  use  = -1;     // 后续引用点四元式编号，-1 = 非待用
    bool live = false;  // 后续是否活跃
};

struct Quad {
    std::string op, arg1, arg2, result;
    // 三个操作数各自的待用信息（只对"变量"操作数有意义）
    UseInfo info1, info2, infoRes;
    int block = -1;     // 属于哪个基本块
};

struct Symbol {
    std::string name;
    int type;           // 0=int 1=double
    int offset;         // [ebp-offset]
    // 待用信息求解时用的工作字段
    int  use  = -1;
    bool live = false;
};

struct Program {
    std::vector<Symbol> symbols;
    std::unordered_map<std::string,int> sym_index;  // name -> symbols 下标
    int temp_count = 0;
    std::vector<Quad> quads;

    // 临时变量在栈帧里分配的 offset（按需分配，记下来复用）
    std::unordered_map<std::string,int> temp_offset;
    int next_temp_offset = 0;   // 临时变量起始 = 所有局部变量总宽度

    bool is_var(const std::string& s) const;     // 是变量(TBn/Tn_x)还是字面量
    bool is_temp(const std::string& s) const;    // 是否临时变量 Tn_x
    Symbol* find(const std::string& name);
};