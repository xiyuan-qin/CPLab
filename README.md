# CPLab — 编译原理实验

山东大学计算机学院《编译原理》课程实验。从源语言出发，依次实现词法分析、语法分析与语法制导翻译、目标代码生成，构成一条简化的编译流水线。

实现语言为 C++17，每个实验均为命令行程序：从标准输入读取，向标准输出写结果。

## 实验内容

| 实验 | 内容 | 所在分支 |
| :--: | :--- | :------: |
| 实验一 | **词法分析**：将源程序切分为单词（token）流，识别非法记号 | `main` |
| 实验二 | **语法分析 + 语法制导翻译**：LR(1) 分析框架，生成四元式中间代码 | `lab2` |
| 实验三 | **目标代码生成**：求待用/活跃信息，分配寄存器，将四元式翻译为 x86 汇编 | `lab3` |

> 各实验相互独立，分别位于不同分支，互不影响。切换分支即可查看对应实验的完整代码。

## 目录结构（`main` 分支 / 实验一）

```
.
├── token.h        # 词法/语法共享的 token 类型定义（type 字符串 + kind 枚举）
├── lexer.h/.cpp   # 词法分析器：tokenize()
├── main.cpp       # 程序入口：读入源码 → 分词 → 输出
├── build.sh       # 构建脚本
├── test.sh        # 批量运行测试样例
├── clean.sh       # 清理产物
└── test/          # 各实验的测试样例（lab1 / lab2 / lab3）
```

## 构建与运行

实验一（`main` 分支）只依赖 `lexer.cpp` 与 `main.cpp`：

```bash
g++ -std=c++17 lexer.cpp main.cpp -o Main
./Main < test/lab1/sample1.txt
```

清理产物：

```bash
bash clean.sh
```

> 实验二、三在各自分支下另有对应的 `build.sh`，按分支内脚本构建即可。

## 实验一：词法分析

支持的语言要素：

- **关键字**：`int` `double` `scanf` `printf` `if` `then` `while` `do`
- **标识符**：字母开头的字母数字串
- **常数**：无符号整数、无符号浮点数
- **运算符**：`= == != < <= > >= + - * / && || !`
- **界符**：`, ; ( ) { }`
- **注释**：`//` 行注释
- **错误检测**：浮点数多个小数点、小数点位于首尾、整数前导零、无法识别的字符

每个单词输出一行：`词素 种别`。若发现词法错误，仅输出第一个错误信息。

### 示例

输入（`test/lab1/sample1.txt`）：

```c
int a, b;
double c=1.2; // This is a comment
scanf(a);
scanf(b);
printf(c);
```

输出：

```
int INTSYM
a IDENT
, COMMA
b IDENT
; SEMICOLON
double DOUBLESYM
c IDENT
= AO
1.2 DOUBLE
; SEMICOLON
scanf SCANFSYM
( BRACE
a IDENT
) BRACE
; SEMICOLON
...
```

## 测试

`test/` 下按实验组织了样例。构建后逐个运行实验一的样例：

```bash
g++ -std=c++17 lexer.cpp main.cpp -o Main
for f in test/lab1/sample*.txt; do echo "== $f =="; ./Main < "$f"; done
```

## 许可证

详见 [LICENSE](LICENSE)。
