#pragma once
#include "ir.h"
#include <string>
#include <vector>

// 获取变量的访问地址 [ebp-offset]；临时变量按需在局部变量之后分配空间。
std::string getAddress(State& st, const std::string& var);

// 局部寄存器分配：为下标 index 处四元式的左值分配寄存器，必要时生成溢出代码。
std::string getReg(State& st, int index);

// 释放寄存器：若 var 不在该块出口活跃集合中，则从描述符里移除。
void releaseReg(State& st, const std::string& var, std::set<std::string>& liveOut);

// 在 RA 中选择“其变量后续引用点最晚”的寄存器（argmax min use）。
std::string findR(State& st, std::vector<std::string>& RA, int index);

int findBlockIndex(State& st, int index);
