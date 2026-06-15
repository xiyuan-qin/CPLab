#include "codegen.h"
#include "blocks.h"
#include "liveness.h"
#include "regalloc.h"
#include <iostream>
#include <map>
#include <string>

// 操作符到目标指令的映射
static const std::map<std::string, std::string> opMap = {
    {"+","add"}, {"-","sub"}, {"*","mul"}, {"/","div"}, {"=","mov"},
    {"<","cmp"}, {">","cmp"}, {"<=","cmp"}, {">=","cmp"}, {"==","cmp"}, {"!=","cmp"},
    {"&&","and"}, {"||","or"}, {"!","not"},
};
static const std::map<std::string, std::string> jThetaMap = {
    {"j<","jl"}, {"j>","jg"}, {"j<=","jle"}, {"j>=","jge"}, {"j==","je"}, {"j!=","jne"},
};
static const std::map<std::string, std::string> cmpMap = {
    {"<","setl"}, {">","setg"}, {"<=","setle"}, {">=","setge"}, {"==","sete"}, {"!=","setne"},
};
static std::string mapGet(const std::map<std::string,std::string>& m, const std::string& k) {
    auto it = m.find(k);
    return it == m.end() ? "" : it->second;
}

// 只有左操作数的四元式：(=,x,-,z) / (!,x,-,z)
static void genForOnlyX(State& st, int index, int blockIndex) {
    Quad q = st.quads[index];
    const std::string& x = q.opnd1.val;
    const std::string& z = q.left.val;
    std::string R = getReg(st, index);
    std::string x1;
    if (isUNum(x)) {
        x1 = x;
        std::cout << "mov " << R << ", " << x1 << "\n";
    } else {
        if (st.Rval[R].find(x) == st.Rval[R].end()) {       // x∉Rval(R) 才装载
            if (!st.Aval[x].reg.empty()) x1 = *st.Aval[x].reg.begin();
            else x1 = getAddress(st, x);
            std::cout << "mov " << R << ", " << x1 << "\n";
        }
        if (q.op.val != "=") std::cout << mapGet(opMap, q.op.val) << " " << R << "\n";  // not R
        releaseReg(st, x, st.liveOut[blockIndex]);
    }
    st.Rval[R].insert(z);
    st.historyInfo[z] = q.left;
    st.Aval[z].reg.insert(R);
    st.Aval[z].mem.clear();
}

// 一般运算四元式 (θ,x,y,z)
static void genForTheta(State& st, int index, int blockIndex) {
    Quad q = st.quads[index];
    const std::string& x = q.opnd1.val;
    const std::string& y = q.opnd2.val;
    const std::string& z = q.left.val;
    std::string Rz = getReg(st, index);

    std::string x1;
    if (x != "-" && !isUNum(x)) {
        if (!st.Aval[x].reg.empty()) x1 = *st.Aval[x].reg.begin();
        else x1 = getAddress(st, x);
    } else x1 = x;

    std::string y1;
    if (y != "-" && !isUNum(y)) {
        if (!st.Aval[y].reg.empty()) y1 = *st.Aval[y].reg.begin();
        else y1 = getAddress(st, y);
    } else y1 = y;

    std::string opi = mapGet(opMap, q.op.val);
    if (x1 == Rz) {
        std::cout << opi << " " << Rz << ", " << y1 << "\n";
        if (opi == "cmp") std::cout << mapGet(cmpMap, q.op.val) << " " << Rz << "\n";
        st.Aval[x].reg.erase(Rz);
    } else {
        std::cout << "mov " << Rz << ", " << x1 << "\n";
        std::cout << opi << " " << Rz << ", " << y1 << "\n";
        if (opi == "cmp") std::cout << mapGet(cmpMap, q.op.val) << " " << Rz << "\n";
    }
    if (y1 == Rz) {
        if (!isUNum(y)) st.Aval[y].reg.erase(Rz);
    }
    st.Rval[Rz] = {z};
    st.historyInfo[z] = q.left;
    st.Aval[z].reg = {Rz};
    st.Aval[z].mem.clear();
    if (!isUNum(x)) releaseReg(st, x, st.liveOut[blockIndex]);
    if (!isUNum(y)) releaseReg(st, y, st.liveOut[blockIndex]);
}

// 读/写
static void genForRorW(State& st, const Quad& q, int blockIndex) {
    if (q.op.val == "W") std::cout << "jmp ?write";
    else std::cout << "jmp ?read";
    std::cout << "(" << getAddress(st, q.left.val) << ")" << "\n";
    if (!isUNum(q.left.val)) releaseReg(st, q.left.val, st.liveOut[blockIndex]);
}

void genCode(State& st) {
    for (size_t blockIndex = 0; blockIndex < st.basicBlocks.size(); ++blockIndex) {
        auto& block = st.basicBlocks[blockIndex];
        if (st.labelFlag[block.front()] == 1) {
            std::cout << "?" + std::to_string(block.front()) + ":" << "\n";
        }
        for (int j : block) {
            if (isTheta(st.quads[j]))        genForTheta(st, j, (int)blockIndex);
            else if (isOnlyX(st.quads[j]))   genForOnlyX(st, j, (int)blockIndex);
            else if (isROrW(st.quads[j]))    genForRorW(st, st.quads[j], (int)blockIndex);
        }
        // 出口活跃变量写回（liveOut 为 set，按字典序）
        for (auto a : st.liveOut[blockIndex]) {
            if (st.Aval[a].mem.find(a) == st.Aval[a].mem.end()) {
                if (!st.Aval[a].reg.empty()) {
                    for (auto reg : st.Aval[a].reg) {
                        if (st.historyInfo[a].live) {
                            std::cout << "mov " << getAddress(st, a) << ", " << reg << "\n";
                        }
                    }
                }
            }
        }
        Quad qini = st.quads[block.back()];
        if (isJ(qini)) {
            std::cout << "jmp ?" << qini.left.val << "\n";
        } else if (isJTheta(qini)) {
            const std::string& x = qini.opnd1.val;
            const std::string& y = qini.opnd2.val;
            const std::string& q = qini.left.val;
            std::string x1 = !st.Aval[x].reg.empty() ? *st.Aval[x].reg.begin() : x;
            std::string y1 = !st.Aval[y].reg.empty() ? *st.Aval[y].reg.begin() : getAddress(st, y);
            if (x1 == x) {
                x1 = getReg(st, block.back());
                std::cout << "mov " << x1 << ", " << getAddress(st, x) << "\n";
            }
            std::cout << "cmp " << x1 << ", " << y1 << "\n";
            std::cout << mapGet(jThetaMap, qini.op.val) << " ?" << q << "\n";
        } else if (isJnz(qini)) {
            const std::string& x = qini.opnd1.val;
            const std::string& q = qini.left.val;
            std::string x1 = !st.Aval[x].reg.empty() ? *st.Aval[x].reg.begin() : x;
            if (x1 == x) {
                x1 = getReg(st, block.back());
                std::cout << "mov " << x1 << ", " << getAddress(st, x) << "\n";
            }
            std::cout << "cmp " << x1 << ", 0" << "\n";
            std::cout << "jne" << " ?" << q << "\n";
        } else if (isEnd(qini)) {
            std::cout << "halt" << "\n";
        }
        st.Rval.clear();
        st.Aval.clear();
    }
}

void run(State& st) {
    st.labelFlag.assign(st.quads.size(), 0);
    getBasicBlock(st);
    computeLiveness(st);
    genCode(st);
}
