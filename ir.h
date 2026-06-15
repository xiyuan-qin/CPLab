#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>

// ===== 数据模型（对齐参考实现 qingzt/C_Compiler_SDU 的 Lab3）=====

// 四元式中的一个项目：值 + 待用信息(use) + 活跃信息(live)
struct QuadItem {
    std::string val;
    int  use  = -1;     // 后续引用点四元式编号，-1 = 非待用
    bool live = false;  // 后续是否活跃
    QuadItem() {}
    QuadItem(const std::string& v) : val(v) {}
};

// 四元式：op opnd1 opnd2 left(左值)
struct Quad {
    QuadItem op, opnd1, opnd2, left;
};

// 符号表项
struct SymItem {
    std::string name, type, value;
    int  offset = -1;       // [ebp-offset]，临时变量分配前为 -1
    int  use    = -1;       // 待用信息工作字段
    bool isTemp = true;     // 默认是临时变量；符号表登记项会置为 false
    bool live   = false;    // 活跃信息工作字段
};

// 地址描述符：值所在的寄存器集合 / 内存集合（mem 为 {var} 表示在其内存单元中）
struct AvalItem {
    std::set<std::string> reg;
    std::set<std::string> mem;
};

// 全局状态
struct State {
    std::map<std::string, SymItem> symbolTable;   // "TBi" -> 符号表项
    int tempCount = 0;                            // 临时变量个数
    std::vector<Quad> quads;                      // 四元式序列

    int InitOffset = 0;   // 符号表中变量的最大偏移量（临时变量分配起点）
    int offset = 0;       // 临时变量分配时的运行偏移量

    std::vector<std::vector<int>> basicBlocks;    // 基本块（四元式下标数组）
    std::vector<int> labelFlag;                   // 每条四元式是否需要生成标号
    std::vector<std::set<std::string>> liveOut;   // 每个基本块出口处的活跃变量集合

    std::map<std::string, AvalItem> Aval;         // 地址描述符
    std::map<std::string, std::set<std::string>> Rval; // 寄存器描述符
    std::map<std::string, QuadItem> historyInfo;  // 变量最近一次定值时的项目信息

    const std::vector<std::string> regs = {"R0", "R1", "R2"};
};

// ===== 四元式类型判定（与参考实现一致）=====

inline bool isTheta(const Quad& q) {
    char c = q.op.val.empty() ? 0 : q.op.val[0];
    return (c=='+'||c=='-'||c=='*'||c=='/'||c=='='||c=='<'||c=='>'||c=='!'||c=='&'||c=='|')
        && (q.opnd2.val != "-");
}
inline bool isJTheta(const Quad& q) {
    return !q.op.val.empty() && q.op.val[0]=='j' && q.op.val!="j" && q.op.val!="jnz";
}
inline bool isJ(const Quad& q)    { return q.op.val == "j"; }
inline bool isJnz(const Quad& q)  { return q.op.val == "jnz"; }
inline bool isROrW(const Quad& q) { return q.op.val == "W" || q.op.val == "R"; }
inline bool isOnlyX(const Quad& q){ return (q.op.val=="="||q.op.val=="!") && q.opnd2.val=="-"; }
inline bool isEnd(const Quad& q)  { return q.op.val == "End"; }
inline bool isUNum(const std::string& s) { return !s.empty() && s[0]>='0' && s[0]<='9'; }
