#include "regalloc.h"
#include <iostream>
#include <algorithm>
#include <climits>

int findBlockIndex(State& st, int index) {
    for (size_t i = 0; i < st.basicBlocks.size(); ++i) {
        if (std::find(st.basicBlocks[i].begin(), st.basicBlocks[i].end(), index)
            != st.basicBlocks[i].end())
            return (int)i;
    }
    return -1;
}

std::string getAddress(State& st, const std::string& var) {
    if (!var.empty() && var[0] == '[') return var;
    auto it = st.symbolTable.find(var);
    if (it != st.symbolTable.end() && it->second.offset != -1) {
        return "[ebp-" + std::to_string(it->second.offset) + "]";   // 已有地址
    }
    // 临时变量：按类型(末字符 i/d)在局部变量之后开辟空间
    if (!var.empty() && var.back() == 'i') {
        st.offset += 4;
        st.symbolTable[var].offset = st.offset;
    } else if (!var.empty() && var.back() == 'd') {
        st.offset += 8;
        st.symbolTable[var].offset = st.offset;
    }
    return "[ebp-" + std::to_string(st.symbolTable[var].offset) + "]";
}

std::string findR(State& st, std::vector<std::string>& RA, int index) {
    std::string res;
    int maxUse = INT_MIN;
    int blockIndex = findBlockIndex(st, index);
    int blockBack = st.basicBlocks[blockIndex].back();
    for (auto& Ri : RA) {
        bool hasFound = false;
        for (int i = index + 1; i <= blockBack; ++i) {
            if (st.Rval[Ri].find(st.quads[i].opnd1.val) != st.Rval[Ri].end()) {
                hasFound = true;
                if (i > maxUse) { maxUse = i; res = Ri; }
                break;
            } else if (st.Rval[Ri].find(st.quads[i].opnd2.val) != st.Rval[Ri].end()) {
                hasFound = true;
                if (i > maxUse) { maxUse = i; res = Ri; }
                break;
            }
        }
        if (!hasFound) {     // Ri 中变量后续不再使用 → 直接选它
            res = Ri;
            break;
        }
    }
    return res;
}

std::string getReg(State& st, int index) {
    Quad q = st.quads[index];
    const std::string& z = q.left.val;
    const std::string& x = q.opnd1.val;
    const std::string& y = q.opnd2.val;

    // if 存在 Ri∈Aval(x) 且 Rval(Ri)={x} 且 (x=z 或 x.live=N) then return Ri
    if (!isUNum(x) && x != "-") {
        for (auto& Ri : st.Aval[x].reg) {
            if (st.Rval[Ri] == std::set<std::string>{x} && (x == z || !q.opnd1.live)) {
                return Ri;
            }
        }
    }
    // if 存在空寄存器 then return
    for (auto& Ri : st.regs) {
        if (st.Rval[Ri].empty()) return Ri;
    }
    // RA = 非空寄存器集合（此处全部非空）
    std::vector<std::string> RA;
    for (auto& Ri : st.regs) {
        if (!st.Rval[Ri].empty()) RA.push_back(Ri);
    }
    if (RA.empty()) RA = st.regs;

    // if 存在 Rj∈RA 且 Rval(Rj) 内变量都已在内存 then Ri=Rj
    std::string Ri;
    bool hasFound = true;
    for (auto& Rj : RA) {
        hasFound = true;
        for (auto& a : st.Rval[Rj]) {
            if (st.Aval[a].mem.find(a) == st.Aval[a].mem.end()) { hasFound = false; break; }
        }
        if (hasFound) { Ri = Rj; break; }
    }
    if (!hasFound) {
        Ri = findR(st, RA, index);   // argmax min use
    }

    // 处理被腾空寄存器 Ri 中的各变量
    for (auto& a : st.Rval[Ri]) {
        if (st.Aval[a].mem.find(a) == st.Aval[a].mem.end() && a != z) {
            std::cout << "mov " << getAddress(st, a) << ", " << Ri << "\n";  // 写回内存
        }
        if (a == x || (a == y && st.Rval[Ri].find(x) != st.Rval[Ri].end())) {
            st.Aval[a].mem = {a};
            st.Aval[a].reg = {Ri};   // 仍保留在 Ri 中，供随后直接使用
        } else {
            st.Aval[a].mem = {a};
            st.Aval[a].reg = {};
        }
    }
    st.Rval[Ri].clear();
    return Ri;
}

void releaseReg(State& st, const std::string& var, std::set<std::string>& liveOut) {
    if (liveOut.find(var) == liveOut.end()) {    // var 不在出口活跃集合
        for (auto& reg : st.Aval[var].reg) {
            st.Rval[reg].erase(var);
        }
        st.Aval[var].reg.clear();
    }
}
