#include "blocks.h"
#include <set>
#include <algorithm>

// 生成标号：标记目标四元式需要建立标号
static void genLable(State& st, int quad) {
    if (st.labelFlag[quad] == 0) {
        st.labelFlag[quad] = 1;
    }
}

void getBasicBlock(State& st) {
    auto& Q = st.quads;
    int n = (int)Q.size();
    std::set<std::vector<int>> blocks;     // 用 set 使基本块按起始下标有序
    std::vector<int> isEnter(n, 0);        // 标记每条四元式是否是入口语句
    if (n > 0) isEnter[0] = 1;

    for (int i = 0; i < n; ++i) {
        if (isJTheta(Q[i]) || isJnz(Q[i])) {          // (jθ,-,-,qj) 或 (jnz,-,-,qj)
            int index = std::stoi(Q[i].left.val);
            isEnter[index] = 1;
            if (i < n - 1) isEnter[i + 1] = 1;        // 下一条也是入口
            genLable(st, index);
        } else if (isJ(Q[i])) {                       // (j,-,-,qj)
            int index = std::stoi(Q[i].left.val);
            isEnter[index] = 1;
            genLable(st, index);
        } else if (Q[i].op.val == "End") {
            isEnter[n - 1] = 1;
        } else if (isROrW(Q[i])) {
            isEnter[i] = 1;
        }
    }

    int i = 0;
    while (i < n) {
        if (isEnter[i]) {
            if (i == n - 1) {
                blocks.insert(std::vector<int>{i});
            }
            if (i + 1 == n) break;
            for (int j = i + 1; j < n; ++j) {
                if (isEnter[j]) {                     // 遇到下一个入口语句
                    std::vector<int> temp;
                    for (int k = i; k < j; ++k) temp.push_back(k);
                    blocks.insert(temp);
                    i = j;
                    break;
                } else if (Q[j].op.val[0] == 'j' || Q[j].op.val == "ret" || Q[j].op.val == "End") {
                    // 遇到转移或停机语句
                    std::vector<int> temp;
                    for (int k = i; k <= j; ++k) temp.push_back(k);
                    blocks.insert(temp);
                    i = j + 1;
                    break;
                }
            }
        } else {
            ++i;
        }
    }
    st.basicBlocks.assign(blocks.begin(), blocks.end());
}
