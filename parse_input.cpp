#include "ir.h"
#include "parse_input.h"
#include <sstream>
#include <vector>

namespace {
// 拆 "(op,arg1,arg2,result)" 里的四个字段
std::vector<std::string> split_quad(const std::string& body) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : body) {
        if (c == ',') { parts.push_back(cur); cur.clear(); }
        else cur += c;
    }
    parts.push_back(cur);
    return parts;
}
}

Program parse_input(const std::string& text) {
    Program prog;
    std::istringstream in(text);
    std::string line;

    // 第一行：符号表项数
    std::getline(in, line);
    int n = std::stoi(line);

    // 符号表 n 行：name type value offset
    int total_width = 0;
    for (int i = 0; i < n; ++i) {
        std::getline(in, line);
        std::istringstream ls(line);
        Symbol s;
        std::string value;
        ls >> s.name >> s.type >> value >> s.offset;
        prog.sym_index[s.name] = (int)prog.symbols.size();
        // 同时建立 TBi -> 符号表 的映射：第 i 个变量就是 TB i
        prog.sym_index["TB" + std::to_string(i)] = (int)prog.symbols.size();
        prog.symbols.push_back(s);
        int w = (s.type == 0 ? 4 : 8);
        total_width = s.offset + w;   // 最后一个变量 offset+width = 总宽度
    }
    prog.next_temp_offset = total_width;

    // 临时变量个数
    std::getline(in, line);
    prog.temp_count = std::stoi(line);

    // 四元式个数
    std::getline(in, line);
    int m = std::stoi(line);

    // m 条四元式："idx: (op,a1,a2,res)"
    for (int i = 0; i < m; ++i) {
        std::getline(in, line);
        auto lp = line.find('(');
        auto rp = line.rfind(')');
        std::string body = line.substr(lp + 1, rp - lp - 1);
        auto parts = split_quad(body);
        Quad q;
        q.op     = parts[0];
        q.arg1   = parts.size() > 1 ? parts[1] : "-";
        q.arg2   = parts.size() > 2 ? parts[2] : "-";
        q.result = parts.size() > 3 ? parts[3] : "-";
        prog.quads.push_back(q);
    }
    return prog;
}