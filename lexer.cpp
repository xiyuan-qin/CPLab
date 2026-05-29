#include "lexer.h"
#include <cctype>
#include <unordered_map>

namespace {

// 关键字表升级：同时存 type 字符串（实验一兼容）和 kind 枚举（实验二用）
struct KwInfo {
    const char* type;
    TokenKind   kind;
};

const std::unordered_map<std::string, KwInfo> KEYWORDS = {
    {"int",    {"INTSYM",    TokenKind::INT}},
    {"double", {"DOUBLESYM", TokenKind::DOUBLE}},
    {"scanf",  {"SCANFSYM",  TokenKind::SCANF}},
    {"printf", {"PRINTFSYM", TokenKind::PRINTF}},
    {"if",     {"IFSYM",     TokenKind::IF}},
    {"then",   {"THENSYM",   TokenKind::THEN}},
    {"while",  {"WHILESYM",  TokenKind::WHILE}},
    {"do",     {"DOSYM",     TokenKind::DO}},
};

constexpr const char* ERR_MULTI_DOT =
    "Malformed number: More than one decimal point in a floating point number.";
constexpr const char* ERR_DOT_EDGE =
    "Malformed number: Decimal point at the beginning or end of a floating point number.";
constexpr const char* ERR_LEAD_ZERO =
    "Malformed number: Leading zeros in an integer.";
constexpr const char* ERR_UNRECOG =
    "Unrecognizable characters.";

inline bool is_digit(char c) { return std::isdigit(static_cast<unsigned char>(c)); }
inline bool is_alpha(char c) { return std::isalpha(static_cast<unsigned char>(c)); }
inline bool is_alnum(char c) { return std::isalnum(static_cast<unsigned char>(c)); }
inline bool is_space(char c) { return std::isspace(static_cast<unsigned char>(c)); }

} // namespace

LexResult tokenize(const std::string& src) {
    LexResult res;
    const int n = static_cast<int>(src.size());
    int i = 0;

    auto set_err = [&](const char* m) { res.error = m; };

    while (i < n) {
        char c = src[i];

        // 1. 空白
        if (is_space(c)) { ++i; continue; }

        // 2. 单行注释 //...
        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            i += 2;
            while (i < n && src[i] != '\n') ++i;
            continue;
        }

        // 3. 多行注释 /* ... */，没配对就吃到结尾
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            i += 2;
            while (i + 1 < n && !(src[i] == '*' && src[i + 1] == '/')) ++i;
            if (i + 1 < n) i += 2;   // 跳过 */
            else i = n;              // 无配对
            continue;
        }

        // 4. 数字（含 ".数字" 开头的情形，以便报错误2）
        if (is_digit(c) || (c == '.' && i + 1 < n && is_digit(src[i + 1]))) {
            std::string num;
            int dots = 0;
            while (i < n && (is_digit(src[i]) || src[i] == '.')) {
                if (src[i] == '.') ++dots;
                num += src[i++];
            }

            // 优先级：1 > 2 > 3
            if (dots >= 2) { set_err(ERR_MULTI_DOT); return res; }
            if (dots == 1) {
                if (num.front() == '.' || num.back() == '.') {
                    set_err(ERR_DOT_EDGE); return res;
                }
                auto pos = num.find('.');
                std::string intp = num.substr(0, pos);
                if (intp.size() > 1 && intp[0] == '0') {
                    set_err(ERR_LEAD_ZERO); return res;
                }
                res.tokens.push_back({num, "DOUBLE", TokenKind::UFLOAT});
            } else {
                if (num.size() > 1 && num[0] == '0') {
                    set_err(ERR_LEAD_ZERO); return res;
                }
                res.tokens.push_back({num, "INT", TokenKind::UINT});
            }
            continue;
        }

        // 5. 标识符 / 关键字
        if (is_alpha(c)) {
            std::string w;
            while (i < n && is_alnum(src[i])) w += src[i++];
            if (auto it = KEYWORDS.find(w); it != KEYWORDS.end())
                res.tokens.push_back({w, it->second.type, it->second.kind});
            else
                res.tokens.push_back({w, "IDENT", TokenKind::ID});
            continue;
        }

        // 6. 运算符 / 界符
        auto push2 = [&](const char* op, const char* ty, TokenKind k) {
            res.tokens.push_back({op, ty, k}); i += 2;
        };
        auto push1 = [&](char op, const char* ty, TokenKind k) {
            res.tokens.push_back({std::string(1, op), ty, k}); ++i;
        };

        switch (c) {
            case '=':
                if (i + 1 < n && src[i + 1] == '=') push2("==", "RO", TokenKind::EQ);
                else push1('=', "AO", TokenKind::ASSIGN_OP);
                break;
            case '>':
                if (i + 1 < n && src[i + 1] == '=') push2(">=", "RO", TokenKind::GE);
                else push1('>', "RO", TokenKind::GT);
                break;
            case '<':
                if (i + 1 < n && src[i + 1] == '=') push2("<=", "RO", TokenKind::LE);
                else push1('<', "RO", TokenKind::LT);
                break;
            case '!':
                if (i + 1 < n && src[i + 1] == '=') push2("!=", "RO", TokenKind::NEQ);
                else push1('!', "LO", TokenKind::NOT);
                break;
            case '|':
                if (i + 1 < n && src[i + 1] == '|') push2("||", "LO", TokenKind::OR);
                else { set_err(ERR_UNRECOG); return res; }   // 单独的 | 是错误4
                break;
            case '&':
                if (i + 1 < n && src[i + 1] == '&') push2("&&", "LO", TokenKind::AND);
                else { set_err(ERR_UNRECOG); return res; }   // 单独的 & 是错误4
                break;
            case '+': push1('+', "PLUS",      TokenKind::PLUS);   break;
            case '-': push1('-', "MINUS",     TokenKind::MINUS);  break;
            case '*': push1('*', "TIMES",     TokenKind::TIMES);  break;
            case '/': push1('/', "DIVISION",  TokenKind::DIVIDE); break;  // 注释已在前面处理
            case ',': push1(',', "COMMA",     TokenKind::COMMA);  break;
            case '(': push1('(', "BRACE",     TokenKind::LPAREN); break;
            case ')': push1(')', "BRACE",     TokenKind::RPAREN); break;
            case '{': push1('{', "BRACE",     TokenKind::LBRACE); break;
            case '}': push1('}', "BRACE",     TokenKind::RBRACE); break;
            case ';': push1(';', "SEMICOLON", TokenKind::SEMICOLON); break;
            default:
                set_err(ERR_UNRECOG); return res;
        }
    }
    return res;
}