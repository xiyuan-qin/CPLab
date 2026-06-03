#pragma once
#include "ir.h"
#include "blocks.h"
#include <vector>
void compute_liveness(Program& prog, const std::vector<Block>& blocks);