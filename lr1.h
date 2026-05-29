#pragma once
#include "grammar.h"
#include <map>
#include <set>
#include <vector>

namespace lr1 {

using grammar::Sym;

// LR(1) 项：[产生式编号 prod, 圆点位置 dot, 向前看符号 lookahead]
struct Item {
    int prod;
    int dot;
    Sym lookahead;
    bool operator<(const Item& o) const {
        if (prod != o.prod)         return prod < o.prod;
        if (dot != o.dot)           return dot < o.dot;
        return lookahead < o.lookahead;
    }
    bool operator==(const Item& o) const {
        return prod == o.prod && dot == o.dot && lookahead == o.lookahead;
    }
};

using ItemSet = std::set<Item>;

// ACTION 表项
enum ActionType { ACT_ERROR, ACT_SHIFT, ACT_REDUCE, ACT_ACCEPT };
struct Action {
    ActionType type = ACT_ERROR;
    int value = 0;   // SHIFT: 目标状态; REDUCE: 产生式编号
};

struct Table {
    int num_states;
    std::vector<std::vector<Action>> action;  // [state][terminal]
    std::vector<std::vector<int>>    go;      // [state][nonterminal] -> state, -1 = 无
};

// 构造整张表（一次性，在程序启动时调用）
Table build_table();

} // namespace lr1