#include "parser.h"
#include "grammar.h"
#include "lr1.h"
#include <vector>

namespace {

grammar::Sym tok_to_term(const Token& t) {
    using grammar::Terminal;
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

} // namespace

ParseResult parse(const std::vector<Token>& tokens) {
    static const lr1::Table& T = []() -> const lr1::Table& {
        static lr1::Table t = lr1::build_table();
        return t;
    }();
    const auto& g = grammar::Grammar::instance();

    // 在 token 流末尾补 END
    std::vector<grammar::Sym> input;
    input.reserve(tokens.size() + 1);
    for (const auto& t : tokens) input.push_back(tok_to_term(t));
    input.push_back(grammar::T_END);

    std::vector<int> state_stack = {0};
    size_t ip = 0;

    while (true) {
        int s = state_stack.back();
        grammar::Sym a = input[ip];
        const lr1::Action& act = T.action[s][a];

        if (act.type == lr1::ACT_SHIFT) {
            state_stack.push_back(act.value);
            ++ip;
        } else if (act.type == lr1::ACT_REDUCE) {
            const auto& p = g.prods[act.value];
            int pop = p.rhs.size();
            for (int i = 0; i < pop; ++i) state_stack.pop_back();
            int s2 = state_stack.back();
            int next = T.go[s2][p.lhs - grammar::NUM_TERMS];
            if (next < 0) return {false, "Syntax Error"};
            state_stack.push_back(next);
            // 语义动作分发会在这里加：dispatch_semantic(act.value, ...);
        } else if (act.type == lr1::ACT_ACCEPT) {
            return {true, ""};
        } else {
            return {false, "Syntax Error"};
        }
    }
}