#include "regalloc.h"
#include <cctype>
#include <climits>
#include <algorithm>

bool RegAlloc::is_temp(const std::string& s) const {
    return s.size() >= 2 && s[0] == 'T' && std::isdigit((unsigned char)s[1]);
}
bool RegAlloc::is_var(const std::string& s) const {
    return !s.empty() && s != "-" && s[0] == 'T';
}

void RegAlloc::clear_descriptors() {
    for (auto& r : Rval) r.clear();
    Aval.clear();
}

std::string RegAlloc::addr(const std::string& var) {
    // 符号表变量 TBn
    if (auto* s = prog.find(var)) {
        return "[ebp-" + std::to_string(s->offset) + "]";
    }
    // 临时变量:按需在局部变量之后分配 offset
    auto it = prog.temp_offset.find(var);
    if (it == prog.temp_offset.end()) {
        int off = prog.next_temp_offset;
        prog.temp_offset[var] = off;
        // 临时变量按 4 字节递增(题目简化,不区分类型宽度)
        prog.next_temp_offset += 4;
        return "[ebp-" + std::to_string(off) + "]";
    }
    return "[ebp-" + std::to_string(it->second) + "]";
}

int RegAlloc::in_reg(const std::string& var) {
    for (int i = 0; i < NUM_REGS; ++i)
        if (Rval[i].count(var)) return i;
    return -1;
}

std::string RegAlloc::operand_loc(const std::string& var) {
    if (!is_var(var)) return var;          // 字面量原样
    int r = in_reg(var);
    if (r >= 0) return reg_name(r);
    return addr(var);                      // 不在寄存器→内存
}

// ===== getReg：对照题目伪代码 =====
int RegAlloc::getReg(const Quad& q, int qidx) {
    const std::string& x = q.arg1;
    const std::string& z = q.result;

    // [伪代码] if 存在 Ri∈Aval(x) 且 Rval(Ri)={x} 且 (x=z 或 x.live=N) then return Ri
    if (is_var(x)) {
        int rx = in_reg(x);
        if (rx >= 0 && Rval[rx].size() == 1 && *Rval[rx].begin() == x) {
            // x.live：用四元式里 x 的活跃信息
            bool x_live = q.info1.live;
            if (x == z || !x_live) return rx;
        }
    }

    // [伪代码] if 存在空 Ri then return Ri
    for (int i = 0; i < NUM_REGS; ++i)
        if (Rval[i].empty()) return i;

    // [伪代码] RA = {Ri | Rval(Ri) 含已在内存的变量}; 若空则 RA=R
    std::vector<int> RA;
    for (int i = 0; i < NUM_REGS; ++i) {
        bool has_in_mem = false;
        for (const auto& a : Rval[i])
            if (Aval[a].in_mem) { has_in_mem = true; break; }
        if (has_in_mem) RA.push_back(i);
    }
    if (RA.empty())
        for (int i = 0; i < NUM_REGS; ++i) RA.push_back(i);

    // [伪代码] if 存在 Rj∈RA 且 Rval(Rj) 中所有 a 都已在内存 then Ri=Rj
    int chosen = -1;
    for (int j : RA) {
        bool all_in_mem = true;
        for (const auto& a : Rval[j])
            if (!Aval[a].in_mem) { all_in_mem = false; break; }
        if (all_in_mem) { chosen = j; break; }
    }
    // [伪代码] else Ri = argmax_{Rj∈RA} ( min_{a∈Rval(Rj)} a.use )
    if (chosen < 0) {
        int best = RA[0], best_score = -1;
        for (int j : RA) {
            int min_use = INT_MAX;
            for (const auto& a : Rval[j]) {
                int u = next_use(a, qidx);    // a 在 qidx 之后的下一个使用点
                min_use = std::min(min_use, u);
            }
            // argmax:选 min_use 最大的(它存的变量最晚才用,腾出来最值)
            if (min_use > best_score) { best_score = min_use; best = j; }
        }
        chosen = best;
    }

    int Ri = chosen;

    // [伪代码] foreach a∈Rval(Ri):
    //   if a 不在内存 且 a≠z then 生成 mov addr(a), Ri
    //   更新 Aval、Rval
    std::vector<std::string> members(Rval[Ri].begin(), Rval[Ri].end());
    for (const auto& a : members) {
        if (!Aval[a].in_mem && a != z) {
            out.push_back("mov " + addr(a) + ", " + reg_name(Ri));
            Aval[a].in_mem = true;
        }
        Aval[a].regs.erase(Ri);
        Rval[Ri].erase(a);
    }
    return Ri;
}

void RegAlloc::releaseReg(const std::string& var, const std::vector<int>& /*liveOut*/) {
    // [伪代码] if var∉liveOut(Bi) 且存在 reg∈Aval(var) then 移除
    // liveOut 判断:临时变量不活跃,符号表变量活跃。这里用 var 是否临时近似。
    if (is_temp(var)) {
        int r = in_reg(var);
        if (r >= 0) {
            Rval[r].erase(var);
            Aval[var].regs.erase(r);
        }
    }
}

int RegAlloc::next_use(const std::string& var, int after) const {
    for (int i = after + 1; i <= block_end; ++i) {
        const Quad& q = prog.quads[i];
        if (q.arg1 == var || q.arg2 == var) return i;
    }
    return INT_MAX;   // 块内不再使用
}