#include "parser.h"
#include "grammar.h"
#include "lr1.h"
#include <iostream>
#include <string>

namespace {

grammar::Sym tok_to_term(const Token& t) {
    switch (t.kind) {
        case TokenKind::UINT:      return grammar::T_UINT;
        case TokenKind::UFLOAT:    return grammar::T_UFLOAT;
        case TokenKind::ID:        return grammar::T_ID;
        case TokenKind::INT:       return grammar::T_INT;
        case TokenKind::DOUBLE:    return grammar::T_DOUBLE;
        case TokenKind::SCANF:     return grammar::T_SCANF;
        case TokenKind::PRINTF:    return grammar::T_PRINTF;
        case TokenKind::IF:        return grammar::T_IF;
        case TokenKind::THEN:      return grammar::T_THEN;
        case TokenKind::WHILE:     return grammar::T_WHILE;
        case TokenKind::DO:        return grammar::T_DO;
        case TokenKind::ASSIGN_OP: return grammar::T_ASSIGN;
        case TokenKind::EQ:        return grammar::T_EQ;
        case TokenKind::NEQ:       return grammar::T_NEQ;
        case TokenKind::LT:        return grammar::T_LT;
        case TokenKind::LE:        return grammar::T_LE;
        case TokenKind::GT:        return grammar::T_GT;
        case TokenKind::GE:        return grammar::T_GE;
        case TokenKind::PLUS:      return grammar::T_PLUS;
        case TokenKind::MINUS:     return grammar::T_MINUS;
        case TokenKind::TIMES:     return grammar::T_TIMES;
        case TokenKind::DIVIDE:    return grammar::T_DIVIDE;
        case TokenKind::AND:       return grammar::T_AND;
        case TokenKind::OR:        return grammar::T_OR;
        case TokenKind::NOT:       return grammar::T_NOT;
        case TokenKind::COMMA:     return grammar::T_COMMA;
        case TokenKind::SEMICOLON: return grammar::T_SEMI;
        case TokenKind::LPAREN:    return grammar::T_LPAREN;
        case TokenKind::RPAREN:    return grammar::T_RPAREN;
        case TokenKind::LBRACE:    return grammar::T_LBRACE;
        case TokenKind::RBRACE:    return grammar::T_RBRACE;
        case TokenKind::END:       return grammar::T_END;
    }
    return grammar::T_END;
}

// 终结符 shift 时构造属性（主要存词素；UFLOAT 转 6 位小数）
Attr make_terminal_attr(const Token& t) {
    Attr a;
    if (t.kind == TokenKind::UFLOAT) {
        a.lexeme = std::to_string(std::stod(t.lexeme));   // "2.4" -> "2.400000"
    } else {
        a.lexeme = t.lexeme;
    }
    return a;
}

// 归约时执行的语义动作。rhs[0] 对应产生式右部最左符号。
Attr dispatch(int prod_id, std::vector<Attr>& rhs, Semantic& sem) {
    Attr act;
    switch (prod_id) {
        case 2:  // SUBPROG -> M VARIABLES STATEMENT
            sem.backpatch(rhs[2].nextlist, sem.nextquad());
            sem.gen("End", "-", "-", "-");
            break;
        case 3:  // M -> ε
            sem.OFFSET = 0;
            break;
        case 4:  // N -> ε
            act.quad = sem.nextquad();
            break;
        case 7:  // T -> int
            act.type = TYPE_INT;    act.width = 4;
            break;
        case 8:  // T -> double
            act.type = TYPE_DOUBLE; act.width = 8;
            break;
        case 9:  // ID -> id
            act.name = rhs[0].lexeme;
            break;
        case 10: // VARIABLE -> T ID
            if (!sem.enter(rhs[1].name, rhs[0].type, sem.OFFSET)) sem.error = true;
            sem.OFFSET += rhs[0].width;
            act.type = rhs[0].type; act.width = rhs[0].width;
            break;
        case 11: // VARIABLE -> VARIABLE , ID
            if (!sem.enter(rhs[2].name, rhs[0].type, sem.OFFSET)) sem.error = true;
            sem.OFFSET += rhs[0].width;
            act.type = rhs[0].type; act.width = rhs[0].width;
            break;
        case 16: // STATEMENT -> { L ; }
            act.nextlist = rhs[1].nextlist;
            break;
        case 19: // ASSIGN -> ID = EXPR
        {
            std::string p = sem.lookup(rhs[0].name, false);   // 左值：不查 assigned
            if (p.empty()) { sem.error = true; }
            else { sem.gen("=", rhs[2].place, "-", p); sem.mark_assigned(rhs[0].name); }
            break;
        }
        case 21: // L -> STATEMENT
            act.nextlist = rhs[0].nextlist;
            break;

        // ===== 数值表达式 =====
        case 22: // EXPR -> EXPR || ORITEM
            act.place = sem.newtemp(TYPE_INT); act.type = TYPE_INT;
            sem.gen("||", rhs[0].place, rhs[2].place, act.place);
            break;
        case 24: // ORITEM -> ORITEM && ANDITEM
            act.place = sem.newtemp(TYPE_INT); act.type = TYPE_INT;
            sem.gen("&&", rhs[0].place, rhs[2].place, act.place);
            break;
        case 27: // ANDITEM -> ! NOITEM
            act.place = sem.newtemp(TYPE_INT); act.type = TYPE_INT;
            sem.gen("!", rhs[1].place, "-", act.place);
            break;
        case 28: // NOITEM -> NOITEM REL RELITEM
            act.place = sem.newtemp(TYPE_INT); act.type = TYPE_INT;
            sem.gen(rhs[1].op, rhs[0].place, rhs[2].place, act.place);
            break;
        case 30: // RELITEM -> RELITEM PLUS_MINUS ITEM
            act.place = sem.newtemp(rhs[0].type); act.type = rhs[0].type;
            sem.gen(rhs[1].op, rhs[0].place, rhs[2].place, act.place);
            break;
        case 33: // ITEM -> ITEM MUL_DIV FACTOR
            act.place = sem.newtemp(rhs[2].type); act.type = rhs[2].type;
            sem.gen(rhs[1].op, rhs[0].place, rhs[2].place, act.place);
            break;
        case 34: // FACTOR -> ID
        {
            std::string p = sem.lookup(rhs[0].name, true);    // 右值：查 assigned
            if (p.empty()) sem.error = true;
            act.place = p;
            act.type = sem.lookup_type(rhs[0].name);
            break;
        }
        case 35: // FACTOR -> UINT
            act.place = sem.newtemp(TYPE_INT); act.type = TYPE_INT;
            sem.gen("=", rhs[0].lexeme, "-", act.place);
            break;
        case 36: // FACTOR -> UFLOAT
            act.place = sem.newtemp(TYPE_DOUBLE); act.type = TYPE_DOUBLE;
            sem.gen("=", rhs[0].lexeme, "-", act.place);
            break;
        case 37: // FACTOR -> ( EXPR )
            act.place = rhs[1].place; act.type = rhs[1].type;
            break;
        case 38: // FACTOR -> PLUS_MINUS FACTOR
            act.place = sem.newtemp(rhs[1].type); act.type = rhs[1].type;
            sem.gen(rhs[0].op, "0", rhs[1].place, act.place);
            break;

        // 单符号传递：EXPR->ORITEM, ORITEM->ANDITEM, ANDITEM->NOITEM,
        // NOITEM->RELITEM, RELITEM->ITEM, ITEM->FACTOR
        case 23: case 25: case 26: case 29: case 31: case 32:
            act.place = rhs[0].place; act.type = rhs[0].type;
            break;

        // ===== 运算符 =====
        case 50: act.op = "+";  break;
        case 51: act.op = "-";  break;
        case 52: act.op = "*";  break;
        case 53: act.op = "/";  break;
        case 54: act.op = "=="; break;
        case 55: act.op = "!="; break;
        case 56: act.op = "<";  break;
        case 57: act.op = "<="; break;
        case 58: act.op = ">";  break;
        case 59: act.op = ">="; break;

        // ===== 读写 =====
        case 61: case 62: // SCANF_BEGIN -> ... ID
        {
            std::string p = sem.lookup(rhs[rhs.size() - 1].name, false); // scanf 给变量赋值
            if (p.empty()) sem.error = true;
            else { sem.gen("R", "-", "-", p); sem.mark_assigned(rhs[rhs.size() - 1].name); }
            break;
        }
        case 64: case 65: // PRINTF_BEGIN -> ... ID
        {
            std::string p = sem.lookup(rhs[rhs.size() - 1].name, true);  // printf 输出，要已赋值
            if (p.empty()) sem.error = true;
            else sem.gen("W", "-", "-", p);
            break;
        }

        // 阶段 E 再填：while / if / L 回填 / B 布尔表达式
        // case 17,18,20,39..49
        // ===== STATEMENT 控制流 =====
        case 17: // STATEMENT -> while N1 B do N2 STATEMENT1
            // backpatch(STATEMENT1.nextlist, N1.quad)
            sem.backpatch(rhs[5].nextlist, rhs[1].quad);
            // backpatch(B.truelist, N2.quad)
            sem.backpatch(rhs[2].truelist, rhs[4].quad);
            // STATEMENT.nextlist = B.falselist
            act.nextlist = rhs[2].falselist;
            // gen(j,-,-,N1.quad)
            sem.gen("j", "-", "-", std::to_string(rhs[1].quad));
            break;
        case 18: // STATEMENT -> if B then N STATEMENT1
            sem.backpatch(rhs[1].truelist, rhs[3].quad);
            act.nextlist = merge(rhs[1].falselist, rhs[4].nextlist);
            break;
        case 20: // L -> L1 ; N STATEMENT
            sem.backpatch(rhs[0].nextlist, rhs[2].quad);
            act.nextlist = rhs[3].nextlist;
            break;

        // ===== 布尔表达式 B =====
        case 39: // B -> B1 || N BORTERM
            sem.backpatch(rhs[0].falselist, rhs[2].quad);
            act.truelist  = merge(rhs[0].truelist, rhs[3].truelist);
            act.falselist = rhs[3].falselist;
            break;
        case 40: // B -> BORTERM
            act.truelist  = rhs[0].truelist;
            act.falselist = rhs[0].falselist;
            break;
        case 41: // BORTERM -> BORTERM1 && N BANDTERM
            sem.backpatch(rhs[0].truelist, rhs[2].quad);
            act.falselist = merge(rhs[0].falselist, rhs[3].falselist);
            act.truelist  = rhs[3].truelist;
            break;
        case 42: // BORTERM -> BANDTERM
            act.truelist  = rhs[0].truelist;
            act.falselist = rhs[0].falselist;
            break;
        case 43: // BANDTERM -> ( B )
            act.truelist  = rhs[1].truelist;
            act.falselist = rhs[1].falselist;
            break;
        case 44: // BANDTERM -> ! BANDTERM1
            act.truelist  = rhs[1].falselist;
            act.falselist = rhs[1].truelist;
            break;
        case 45: // BANDTERM -> BFACTOR REL BFACTOR
            act.truelist  = mklist(sem.nextquad());
            act.falselist = mklist(sem.nextquad() + 1);
            sem.gen("j" + rhs[1].op, rhs[0].place, rhs[2].place, "0");
            sem.gen("j", "-", "-", "0");
            break;
        case 46: // BANDTERM -> BFACTOR
            act.truelist  = mklist(sem.nextquad());
            act.falselist = mklist(sem.nextquad() + 1);
            sem.gen("jnz", rhs[0].place, "-", "0");
            sem.gen("j", "-", "-", "0");
            break;

        // ===== 布尔因子 BFACTOR =====
        case 47: // BFACTOR -> UINT
            act.place = sem.newtemp(TYPE_INT); act.type = TYPE_INT;
            sem.gen("=", rhs[0].lexeme, "-", act.place);
            break;
        case 48: // BFACTOR -> UFLOAT
            act.place = sem.newtemp(TYPE_DOUBLE); act.type = TYPE_DOUBLE;
            sem.gen("=", rhs[0].lexeme, "-", act.place);
            break;
        case 49:
        {
            std::string p = sem.lookup(rhs[0].name, true);
            if (p.empty()) {
                sem.error = true;
            }
            act.place = p;
            act.type  = sem.lookup_type(rhs[0].name);
            break;
        }

        default:
            break;
    }
    return act;
}

} // namespace

ParseResult parse(const std::vector<Token>& tokens) {
    static lr1::Table T = lr1::build_table();
    const auto& g = grammar::Grammar::instance();

    // 输入末尾补 END
    std::vector<Token> input = tokens;
    input.push_back({"#", "", TokenKind::END});

    Semantic sem;
    std::vector<int> state_stack = {0};
    std::vector<Attr> attr_stack;
    size_t ip = 0;

    while (true) {
        int s = state_stack.back();
        grammar::Sym a = tok_to_term(input[ip]);
        const lr1::Action& act = T.action[s][a];

        if (act.type == lr1::ACT_SHIFT) {
            state_stack.push_back(act.value);
            attr_stack.push_back(make_terminal_attr(input[ip]));
            ++ip;
        } else if (act.type == lr1::ACT_REDUCE) {
            const auto& p = g.prods[act.value];
            int k = static_cast<int>(p.rhs.size());
            std::vector<Attr> rhs(k);
            for (int j = k - 1; j >= 0; --j) {
                rhs[j] = attr_stack.back();
                attr_stack.pop_back();
                state_stack.pop_back();
            }
            Attr lhs = dispatch(act.value, rhs, sem);
            if (sem.error) {
                return {false, "Syntax Error", {}, {}, 0};
            }

            int s2 = state_stack.back();
            int go = T.go[s2][p.lhs - grammar::NUM_TERMS];
            if (go < 0) {
                return {false, "Syntax Error", {}, {}, 0};
            }
            state_stack.push_back(go);
            attr_stack.push_back(lhs);
        } else if (act.type == lr1::ACT_ACCEPT) {
            return {true, "", sem.symbols, sem.quads, sem.temp_count};
        } else {
            return {false, "Syntax Error", {}, {}, 0};
        }
    }
}