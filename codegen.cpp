#include "codegen.h"
#include "regalloc.h"
#include <algorithm>
#include <cctype>

namespace {

bool is_variable(const std::string& s) { return !s.empty() && s != "-" && s[0]=='T'; }
bool is_temp_v(const std::string& s) {
    return s.size()>=2 && s[0]=='T' && std::isdigit((unsigned char)s[1]);
}

// 四元式 op -> set 指令后缀 / 跳转指令
std::string set_suffix(const std::string& op) {
    if (op=="==") return "sete";
    if (op=="!=") return "setne";
    if (op=="<")  return "setl";
    if (op=="<=") return "setle";
    if (op==">")  return "setg";
    if (op==">=") return "setge";
    return "";
}
std::string jmp_for(const std::string& jop) {
    // jop 形如 j== j!= j< j<= j> j>= jnz
    std::string rel = jop.substr(1);
    if (rel=="==") return "je";
    if (rel=="!=") return "jne";
    if (rel=="<")  return "jl";
    if (rel=="<=") return "jle";
    if (rel==">")  return "jg";
    if (rel==">=") return "jge";
    if (rel=="nz") return "jne";
    return "jmp";
}
std::string arith_instr(const std::string& op) {
    if (op=="+") return "add";
    if (op=="-") return "sub";
    if (op=="*") return "mul";
    if (op=="/") return "div";
    if (op=="&&") return "and";
    if (op=="||") return "or";
    return "";
}
bool is_cond_jump(const std::string& op) {
    return op.size()>=2 && op[0]=='j' && op!="j";
}
bool is_compute(const std::string& op) {
    // 产生 set 的比较运算 或 算术/逻辑运算
    return op=="+"||op=="-"||op=="*"||op=="/"||op=="&&"||op=="||"||op=="!"
        || op=="=="||op=="!="||op=="<"||op=="<="||op==">"||op==">=";
}

} // namespace

std::vector<std::string> generate_code(Program& prog, std::vector<Block>& blocks) {
    std::vector<std::string> out;
    RegAlloc ra(prog, out);
    std::vector<bool> labelFlag(blocks.size(), false);

    // 块号 -> 块起始四元式编号,用于打标号
    auto block_of = [&](int quad_idx) { return prog.quads[quad_idx].block; };
    auto gen_label = [&](int target_quad) {
        int blk = block_of(target_quad);
        if (!labelFlag[blk]) {
            // 标号用目标四元式编号
        }
        // 题目:标号用四元式编号 ?N: 。但要避免重复——按块去重
        // 这里标号文本用目标 quad 编号
    };

    // 收集所有跳转目标四元式编号 -> 它们所属的块
    std::vector<bool> block_is_target(blocks.size(), false);
    for (const auto& q : prog.quads) {
        if (q.op == "j" || is_cond_jump(q.op)) {
            int target = std::stoi(q.result);
            int blk = prog.quads[target].block;
            block_is_target[blk] = true;
        }
    }

    for (size_t bi = 0; bi < blocks.size(); ++bi) {
        Block& blk = blocks[bi];

        if (block_is_target[bi]) {
            out.push_back("?" + std::to_string(blk.start) + ":");
        }

        ra.clear_descriptors();
        ra.block_start = blk.start;
        ra.block_end   = blk.end;


        // 若本块是某跳转目标,块首可能要打标号——延后到生成跳转时统一处理
        // 这里先检查:本块起始四元式是否被标记需要标号(用 labelFlag)
        // 我们改为:谁跳到这,谁负责打标号(见块尾)。但标号必须出现在目标块之前。
        // 实现:先扫一遍所有跳转,记录哪些 quad 是目标,需要标号。

        for (int i = blk.start; i <= blk.end; ++i) {
            Quad& q = prog.quads[i];
            const std::string& op = q.op;

            if (op == "R") {
                out.push_back("jmp ?read(" + ra.addr(q.result) + ")");
            } else if (op == "W") {
                out.push_back("jmp ?write(" + ra.addr(q.result) + ")");
            } else if (op == "End") {
                out.push_back("halt");
            } else if (op == "=") {
                // (=,src,-,dest):mov R, src ; R 是新分配给 dest 的寄存器
                int R = ra.getReg(q, i);
                std::string srcloc = ra.operand_loc(q.arg1);
                // 如果 src 已在 R 中则不生成
                if (!(is_variable(q.arg1) && ra.in_reg(q.arg1) == R)) {
                    out.push_back("mov " + ra.reg_name(R) + ", " + srcloc);
                }
                // 更新描述符:dest 在 R 中
                ra.Rval[R].clear();
                ra.Rval[R].insert(q.result);
                ra.Aval[q.result].regs = {R};
                ra.Aval[q.result].in_mem = false;
                ra.releaseReg(q.arg1, {});
            } else if (op == "!") {
                int R = ra.getReg(q, i);
                std::string xloc = ra.operand_loc(q.arg1);
                if (!(is_variable(q.arg1) && ra.in_reg(q.arg1) == R))
                    out.push_back("mov " + ra.reg_name(R) + ", " + xloc);
                out.push_back("not " + ra.reg_name(R));
                ra.Rval[R].clear(); ra.Rval[R].insert(q.result);
                ra.Aval[q.result].regs = {R}; ra.Aval[q.result].in_mem = false;
                ra.releaseReg(q.arg1, {});
            } else if (is_compute(op) && i != blk.end) {
                // 块中间的运算四元式(算术/逻辑/比较)
                int R = ra.getReg(q, i);
                std::string xloc = ra.operand_loc(q.arg1);
                std::string yloc = ra.operand_loc(q.arg2);
                int rx = is_variable(q.arg1) ? ra.in_reg(q.arg1) : -1;
                if (rx == R) {
                    // x 已在 R:直接 op R, y
                    ra.Aval[q.arg1].regs.erase(R);
                } else {
                    out.push_back("mov " + ra.reg_name(R) + ", " + xloc);
                }
                std::string suf = set_suffix(op);
                if (!suf.empty()) {
                    // 比较运算:cmp R, y ; setX R
                    out.push_back("cmp " + ra.reg_name(R) + ", " + yloc);
                    out.push_back(suf + " " + ra.reg_name(R));
                } else {
                    out.push_back(arith_instr(op) + " " + ra.reg_name(R) + ", " + yloc);
                }
                int ry = is_variable(q.arg2) ? ra.in_reg(q.arg2) : -1;
                if (ry == R) ra.Aval[q.arg2].regs.erase(R);
                ra.Rval[R].clear(); ra.Rval[R].insert(q.result);
                ra.Aval[q.result].regs = {R}; ra.Aval[q.result].in_mem = false;
                ra.releaseReg(q.arg1, {});
                ra.releaseReg(q.arg2, {});
            }
            // 块尾跳转在下面统一处理
        }

        // === 块尾:写回出口活跃变量(字典序) ===
        std::vector<std::string> live_vars;
        for (auto& [var, ad] : ra.Aval) {
            if (!is_temp_v(var) && !ad.in_mem && !ad.regs.empty())
                live_vars.push_back(var);
        }
        std::sort(live_vars.begin(), live_vars.end());
        for (auto& var : live_vars) {
            int r = *ra.Aval[var].regs.begin();
            out.push_back("mov " + ra.addr(var) + ", " + ra.reg_name(r));
            ra.Aval[var].in_mem = true;
        }

        // === 块尾跳转 ===
        Quad& last = prog.quads[blk.end];
        if (last.op == "j") {
            out.push_back("jmp ?" + last.result);
        } else if (is_cond_jump(last.op)) {
            // jθ / jnz:已经在循环里没处理(因为 i==blk.end 跳过了 compute)
            // 这里生成比较+条件跳转
            std::string xloc = ra.operand_loc(last.arg1);
            int rx = is_variable(last.arg1) ? ra.in_reg(last.arg1) : -1;
            std::string Rx;
            if (rx < 0) {
                int R = ra.getReg(last, blk.end);
                out.push_back("mov " + ra.reg_name(R) + ", " + xloc);
                Rx = ra.reg_name(R);
            } else {
                Rx = ra.reg_name(rx);
            }
            if (last.op == "jnz") {
                out.push_back("cmp " + Rx + ", 0");
                out.push_back("jne ?" + last.result);
            } else {
                std::string yloc = ra.operand_loc(last.arg2);
                out.push_back("cmp " + Rx + ", " + yloc);
                out.push_back(jmp_for(last.op) + " ?" + last.result);
            }
        }
    }

    // === 插入标号 ===
    // 收集所有跳转目标 quad 编号,在对应输出位置前插 ?N:
    // 由于 out 是线性的,需要在生成时记录"每个块第一行的 out 下标"
    // —— 这部分见下方说明,需要重构为带标号的版本

    return out;
}