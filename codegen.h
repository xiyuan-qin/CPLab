#pragma once
#include "ir.h"
#include "blocks.h"
#include <string>
#include <vector>
std::vector<std::string> generate_code(Program& prog, std::vector<Block>& blocks);