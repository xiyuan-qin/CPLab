#pragma once
#include "ir.h"

// 生成目标代码（直接输出到 std::cout）。
void genCode(State& st);

// 完整流程：划分基本块 → 求活跃信息 → 生成代码。
void run(State& st);
