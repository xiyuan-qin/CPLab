#include "semantic.h"

bool Semantic::enter(const std::string& name, int type, int offset) {
    if (sym_index.count(name)) return false;     // 重复定义
    sym_index[name] = static_cast<int>(symbols.size());
    symbols.push_back({name, type, offset, false});
    return true;
}

std::string Semantic::lookup(const std::string& name, bool check_assigned) {
    auto it = sym_index.find(name);
    if (it == sym_index.end()) { error = true; return ""; }            // 未定义
    if (check_assigned && !symbols[it->second].assigned) {             // 定义但未赋值
        error = true; return "";
    }
    return "TB" + std::to_string(it->second);
}

int Semantic::lookup_type(const std::string& name) {
    auto it = sym_index.find(name);
    return it == sym_index.end() ? TYPE_INT : symbols[it->second].type;
}

void Semantic::mark_assigned(const std::string& name) {
    auto it = sym_index.find(name);
    if (it != sym_index.end()) symbols[it->second].assigned = true;
}

std::string Semantic::newtemp(int type) {
    return "T" + std::to_string(temp_count++) + (type == TYPE_INT ? "_i" : "_d");
}

void Semantic::gen(const std::string& op, const std::string& a,
                const std::string& b, const std::string& r) {
    quads.push_back({op, a, b, r});
}

void Semantic::backpatch(const std::vector<int>& list, int target) {
    for (int q : list) quads[q].result = std::to_string(target);
}

std::vector<int> mklist(int q) { return {q}; }
std::vector<int> mklist()      { return {}; }
std::vector<int> merge(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> r = a;
    r.insert(r.end(), b.begin(), b.end());
    return r;
}