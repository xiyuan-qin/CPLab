#include "lr1.h"
#include <queue>
#include <iostream>

namespace lr1 {

using namespace grammar;

ItemSet closure(const ItemSet& I) {
    const auto& g = Grammar::instance();
    ItemSet result = I;
    bool changed = true;
    while (changed) {
        changed = false;
        ItemSet to_add;
        for (const auto& it : result) {
            const auto& p = g.prods[it.prod];
            if (it.dot >= (int)p.rhs.size()) continue;
            Sym B = p.rhs[it.dot];
            if (!is_nonterminal(B) && B != g.aug_start) continue;

            // β = p.rhs[dot+1 ..]，a = lookahead
            // 对每个 B -> γ 产生式，向其加入新项 [B -> .γ, b]，b ∈ FIRST(βa)
            std::vector<Sym> beta_a(p.rhs.begin() + it.dot + 1, p.rhs.end());
            beta_a.push_back(it.lookahead);

            // first_of 把 T_END 当 ε
            auto firsts = g.first_of(beta_a);
            // 但 lookahead 是真实终结符，T_END 在这里就是 #，不当 ε
            // 我们在 first_of 里 ε 也用 T_END，需要小心处理：
            // beta_a 末尾是 lookahead 本身，所以即使前缀全 nullable
            // 最后也会从 lookahead 那里拿到真终结符。
            // 把 ε(=T_END) 替换掉的逻辑在 first_of 里已经做了。

            for (int pid : g.prods_of[B]) {
                for (Sym b : firsts) {
                    Item ni{pid, 0, b};
                    if (!result.count(ni)) to_add.insert(ni);
                }
            }
        }
        if (!to_add.empty()) {
            for (auto& x : to_add) result.insert(x);
            changed = true;
        }
    }
    return result;
}

ItemSet goto_set(const ItemSet& I, Sym X) {
    ItemSet J;
    const auto& g = Grammar::instance();
    for (const auto& it : I) {
        const auto& p = g.prods[it.prod];
        if (it.dot < (int)p.rhs.size() && p.rhs[it.dot] == X) {
            J.insert({it.prod, it.dot + 1, it.lookahead});
        }
    }
    return closure(J);
}

Table build_table() {
    const auto& g = Grammar::instance();
    std::vector<ItemSet> states;
    std::map<ItemSet, int> state_id;

    // 初始项：[S' -> .PROG, #]
    ItemSet I0 = closure({{g.aug_prod_id, 0, T_END}});
    states.push_back(I0);
    state_id[I0] = 0;

    std::queue<int> work;
    work.push(0);
    while (!work.empty()) {
        int i = work.front(); work.pop();
        // 对所有可能的下一符号 X 试 goto
        std::set<Sym> nexts;
        for (const auto& it : states[i]) {
            const auto& p = g.prods[it.prod];
            if (it.dot < (int)p.rhs.size()) nexts.insert(p.rhs[it.dot]);
        }
        for (Sym X : nexts) {
            ItemSet J = goto_set(states[i], X);
            if (J.empty()) continue;
            auto it = state_id.find(J);
            if (it == state_id.end()) {
                int nid = states.size();
                states.push_back(J);
                state_id[J] = nid;
                work.push(nid);
            }
        }
    }

    int n = states.size();
    Table T;
    T.num_states = n;
    T.action.assign(n, std::vector<Action>(NUM_TERMS));
    T.go.assign(n, std::vector<int>(NUM_NONTERMS + 1, -1));

    for (int i = 0; i < n; ++i) {
        const auto& I = states[i];
        // SHIFT / GOTO
        std::set<Sym> nexts;
        for (const auto& it : I) {
            const auto& p = g.prods[it.prod];
            if (it.dot < (int)p.rhs.size()) nexts.insert(p.rhs[it.dot]);
        }
        for (Sym X : nexts) {
            ItemSet J = goto_set(I, X);
            int j = state_id[J];
            if (is_terminal(X)) {
                T.action[i][X] = {ACT_SHIFT, j};
            } else if (is_nonterminal(X)) {
                T.go[i][X - NUM_TERMS] = j;
            }
        }
        // REDUCE / ACCEPT
        for (const auto& it : I) {
            const auto& p = g.prods[it.prod];
            if (it.dot == (int)p.rhs.size()) {
                if (p.lhs == g.aug_start) {
                    T.action[i][T_END] = {ACT_ACCEPT, 0};
                } else {
                    // 简化：直接覆盖，假设文法无冲突
                    // 严谨版应该检测 shift-reduce / reduce-reduce 冲突
                    if (T.action[i][it.lookahead].type == ACT_ERROR) {
                        T.action[i][it.lookahead] = {ACT_REDUCE, it.prod};
                    }
                }
            }
        }
    }

    // 调试输出（写完后可注释掉）
    std::cerr << "[lr1] " << n << " states built.\n";

    return T;
}

} // namespace lr1