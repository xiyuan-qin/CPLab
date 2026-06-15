#pragma once
#include "ir.h"
#include <istream>

// 读取输入，填充 State（符号表、临时变量个数、四元式）。
// 若首行为 "Syntax Error"，直接输出 halt 并退出。
void input(State& st, std::istream& in);
