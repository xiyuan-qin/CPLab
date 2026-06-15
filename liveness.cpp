#include "liveness.h"

// 求解单个基本块的待用/活跃信息，返回出口处的活跃变量集合
static std::set<std::string> getUseInfo(State& st, const std::vector<int>& block) {
    std::set<std::string> res;   // 当前块出口处的活跃变量集合
    // 初始化：use 置为非待用；非临时变量在出口处活跃
    for (int idx : block) {
        const std::string& x = st.quads[idx].opnd1.val;
        const std::string& y = st.quads[idx].opnd2.val;
        const std::string& z = st.quads[idx].left.val;
        if (!x.empty() && x[0] == 'T') {
            st.symbolTable[x].use = -1;
            if (!st.symbolTable[x].isTemp) { st.symbolTable[x].live = true; res.insert(x); }
        }
        if (!y.empty() && y[0] == 'T') {
            st.symbolTable[y].use = -1;
            if (!st.symbolTable[y].isTemp) { st.symbolTable[y].live = true; res.insert(y); }
        }
        if (!z.empty() && z[0] == 'T') {
            st.symbolTable[z].use = -1;
            if (!st.symbolTable[z].isTemp) { st.symbolTable[z].live = true; res.insert(z); }
        }
    }
    // 逆序扫描，回填四元式各项的 use/live
    for (int i = (int)block.size() - 1; i >= 0; --i) {
        int index = block[i];
        Quad& q = st.quads[index];
        if (!q.left.val.empty() && q.left.val[0] == 'T') {
            q.left.use  = st.symbolTable[q.left.val].use;
            q.left.live = st.symbolTable[q.left.val].live;
            st.symbolTable[q.left.val].live = false;
            st.symbolTable[q.left.val].use  = -1;
        }
        if (!q.opnd1.val.empty() && q.opnd1.val[0] == 'T') {
            q.opnd1.use  = st.symbolTable[q.opnd1.val].use;
            q.opnd1.live = st.symbolTable[q.opnd1.val].live;
            st.symbolTable[q.opnd1.val].live = true;
            st.symbolTable[q.opnd1.val].use  = index;
        }
        if (!q.opnd2.val.empty() && q.opnd2.val[0] == 'T') {
            q.opnd2.use  = st.symbolTable[q.opnd2.val].use;
            q.opnd2.live = st.symbolTable[q.opnd2.val].live;
            st.symbolTable[q.opnd2.val].live = true;
            st.symbolTable[q.opnd2.val].use  = index;
        }
    }
    return res;
}

void computeLiveness(State& st) {
    st.liveOut.resize(st.basicBlocks.size());
    for (size_t i = 0; i < st.basicBlocks.size(); ++i) {
        st.liveOut[i] = getUseInfo(st, st.basicBlocks[i]);
    }
}
