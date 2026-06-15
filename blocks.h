#pragma once
#include "ir.h"

// 划分基本块，填充 st.basicBlocks，并通过 genLable 设置 st.labelFlag。
void getBasicBlock(State& st);
