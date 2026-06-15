#pragma once
#include "ir.h"

// 求解每个基本块的待用/活跃信息，填充四元式各项的 use/live，
// 并把每个基本块出口处的活跃变量集合写入 st.liveOut。
void computeLiveness(State& st);
