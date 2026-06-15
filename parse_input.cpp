#include "parse_input.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cstdlib>

// 读入符号表
static void getInputForSymbolTable(State& st, int len, std::istream& in) {
    std::string line;
    for (int i = 0; i < len; ++i) {
        std::getline(in, line);
        std::istringstream ss(line);
        SymItem item;
        item.isTemp = false;                 // 符号表登记项是非临时变量
        ss >> item.name >> item.type >> item.value >> item.offset;
        st.InitOffset = std::max(st.InitOffset, item.offset);
        st.symbolTable["TB" + std::to_string(i)] = item;
    }
}

// 读入四元式序列
static void getInputForQuardruples(State& st, int len, std::istream& in) {
    st.quads.resize(len);
    std::string line;
    for (int i = 0; i < len; ++i) {
        std::getline(in, line);
        // 形如 "idx: (op,a1,a2,res)"
        line = line.substr(line.find(':') + 2);     // 去掉 "idx: "
        line = line.substr(1, line.find(')') - 1);  // 去掉首括号与尾括号
        std::istringstream ss(line);
        std::vector<std::string> tokens(4);
        int j = 0;
        while (j < 4 && std::getline(ss, tokens[j], ',')) ++j;
        st.quads[i].op    = tokens[0];
        st.quads[i].opnd1 = tokens[1];
        st.quads[i].opnd2 = tokens[2];
        st.quads[i].left  = tokens[3];
    }
}

void input(State& st, std::istream& in) {
    std::string line;
    std::getline(in, line);
    if (line == "Syntax Error") {
        std::cout << "halt" << std::endl;
        std::exit(0);
    }
    int symbolTableSize = std::stoi(line);
    getInputForSymbolTable(st, symbolTableSize, in);

    std::getline(in, line);
    st.tempCount = std::stoi(line);

    std::getline(in, line);
    int quardruplesSize = std::stoi(line);
    getInputForQuardruples(st, quardruplesSize, in);

    st.offset = st.InitOffset;   // 临时变量从符号表最大偏移量之后开始分配
}
