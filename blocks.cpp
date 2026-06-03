#include "blocks.h"
#include <set>

namespace {
bool is_cond_jump(const std::string& op) {
    // jθ：j 开头且不是单独的 "j"，也不是 jnz 之外…… jnz 也算条件跳转
    return op.size() >= 2 && op[0] == 'j' && op != "j";
}
}

std::vector<Block> partition_blocks(Program& prog) {
    auto& Q = prog.quads;
    int n = (int)Q.size();
    std::set<int> leaders;   // 入口语句集合
    if (n > 0) leaders.insert(0);

    for (int i = 0; i < n; ++i) {
        const std::string& op = Q[i].op;
        // 跳转目标是入口；条件跳转的下一条也是入口
        if (is_cond_jump(op)) {            // jnz / j== / j< ...
            int target = std::stoi(Q[i].result);
            leaders.insert(target);
            if (i + 1 < n) leaders.insert(i + 1);
        } else if (op == "j") {            // 无条件跳转
            int target = std::stoi(Q[i].result);
            leaders.insert(target);
            if (i + 1 < n) leaders.insert(i + 1);
        } else if (op == "R" || op == "W") {
            leaders.insert(i);             // R/W 自成入口
        }
    }

    // 按 leaders 切块
    std::vector<int> ls(leaders.begin(), leaders.end());
    std::vector<Block> blocks;
    for (size_t k = 0; k < ls.size(); ++k) {
        int start = ls[k];
        int end   = (k + 1 < ls.size()) ? ls[k + 1] - 1 : n - 1;
        blocks.push_back({start, end});
    }

    // 给每条四元式标记块号
    for (size_t b = 0; b < blocks.size(); ++b)
        for (int i = blocks[b].start; i <= blocks[b].end; ++i)
            Q[i].block = (int)b;

    return blocks;
}