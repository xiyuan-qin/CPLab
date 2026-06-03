#include "liveness.h"

namespace {
// 给某个操作数（变量）取符号表工作字段；临时变量也要进符号表管理
// 这里用一个临时的 work map 统一处理 TBn 和 Tn_x
struct Work {
    std::unordered_map<std::string, UseInfo> tab;
    UseInfo get(const std::string& v) {
        auto it = tab.find(v);
        return it == tab.end() ? UseInfo{} : it->second;
    }
    void set(const std::string& v, int use, bool live) {
        tab[v] = {use, live};
    }
};

bool is_temp(const std::string& s) {
    return s.size() >= 2 && s[0] == 'T' && std::isdigit((unsigned char)s[1]);
}
bool is_variable(const std::string& s) {
    return !s.empty() && s != "-" && s[0] == 'T';   // TBn 或 Tn_x
}
}

void compute_liveness(Program& prog, const std::vector<Block>& blocks) {
    for (const auto& blk : blocks) {
        Work w;
        // 初始化：块内出现的变量，出口活跃性按规则设
        for (int i = blk.start; i <= blk.end; ++i) {
            const Quad& q = prog.quads[i];
            for (const std::string* s : {&q.arg1, &q.arg2, &q.result}) {
                if (is_variable(*s)) {
                    bool live_out = !is_temp(*s);   // 非临时→出口活跃
                    w.set(*s, -1, live_out);
                }
            }
        }
        // 逆序扫描
        for (int i = blk.end; i >= blk.start; --i) {
            Quad& q = prog.quads[i];
            const std::string &x = q.arg1, &y = q.arg2, &z = q.result;

            // 1. z：先把符号表当前状态记到四元式
            if (is_variable(z)) {
                q.infoRes = w.get(z);
                w.set(z, -1, false);    // z 被定值，之前它是死的
            }
            // 2. x, y：记状态，再置 use=当前编号, live=Y
            if (is_variable(x)) {
                q.info1 = w.get(x);
                w.set(x, i, true);
            }
            if (is_variable(y)) {
                q.info2 = w.get(y);
                w.set(y, i, true);
            }
        }
    }
}