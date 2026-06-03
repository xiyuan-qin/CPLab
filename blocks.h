#pragma once
#include "ir.h"
#include <vector>
struct Block { int start, end; };   // [start, end] 闭区间，四元式下标
std::vector<Block> partition_blocks(Program& prog);