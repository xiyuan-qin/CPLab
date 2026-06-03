#pragma once
#include "ir.h"
#include <array>
#include <set>
#include <string>
#include <vector>

constexpr int NUM_REGS = 3;   // R0 R1 R2

struct AddrDesc {
    std::set<int> regs;    // 值所在的寄存器编号集合
    bool in_mem = false;   // 值是否在内存
};

class RegAlloc {
public:
    Program& prog;
    std::array<std::set<std::string>, NUM_REGS> Rval;     // 每个寄存器存的变量
    std::unordered_map<std::string, AddrDesc> Aval;       // 变量地址描述符
    std::vector<std::string>& out;   // 输出汇编行

    RegAlloc(Program& p, std::vector<std::string>& o) : prog(p), out(o) {}

    void clear_descriptors();                 // 基本块开始时清空
    std::string reg_name(int i) const { return "R" + std::to_string(i); }
    std::string addr(const std::string& var); // 变量的 [ebp-offset]
    bool is_var(const std::string& s) const;
    bool is_temp(const std::string& s) const;

    int  getReg(const Quad& q, int qidx);     // 为 q.result 分配寄存器
    void releaseReg(const std::string& var, const std::vector<int>& liveOut);

    // 取变量当前所在：返回寄存器名(在寄存器里)或内存地址
    std::string operand_loc(const std::string& var);
    // 变量是否在某寄存器,返回寄存器号,-1 表示不在
    int in_reg(const std::string& var);

    int block_start = 0, block_end = 0;   // 当前基本块范围
    int next_use(const std::string& var, int after) const;  // var 在 after 之后的下一个使用点
};