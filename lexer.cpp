#include "lexer.h"
#include <cctype>
#include <unordered_map>

namespace {

const std::unordered_map<std::string, std::string> KEYWORDS = {
    {"int",    "INTSYM"},    {"double", "DOUBLESYM"},
    {"scanf",  "SCANFSYM"},  {"printf", "PRINTFSYM"},
    {"if",     "IFSYM"},     {"then",   "THENSYM"},
    {"while",  "WHILESYM"},  {"do",     "DOSYM"},
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

        // 4. 数字（含 ".数字" 开头的情形，让它走数字分支以便报错误2）
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
                res.tokens.push_back({num, "DOUBLE"});
            } else {
                if (num.size() > 1 && num[0] == '0') {
                    set_err(ERR_LEAD_ZERO); return res;
                }
                res.tokens.push_back({num, "INT"});
            }
            continue;
        }

        // 5. 标识符 / 关键字
        if (is_alpha(c)) {
            std::string w;
            while (i < n && is_alnum(src[i])) w += src[i++];
            if (auto it = KEYWORDS.find(w); it != KEYWORDS.end())
                res.tokens.push_back({w, it->second});
            else
                res.tokens.push_back({w, "IDENT"});
            continue;
        }

        // 6. 运算符 / 界符
        auto push2 = [&](const char* op, const char* ty) {
            res.tokens.push_back({op, ty}); i += 2;
        };
        auto push1 = [&](char op, const char* ty) {
            res.tokens.push_back({std::string(1, op), ty}); ++i;
        };

        switch (c) {
            case '=':
                if (i + 1 < n && src[i + 1] == '=') push2("==", "RO");
                else push1('=', "AO");
                break;
            case '>':
                if (i + 1 < n && src[i + 1] == '=') push2(">=", "RO");
                else push1('>', "RO");
                break;
            case '<':
                if (i + 1 < n && src[i + 1] == '=') push2("<=", "RO");
                else push1('<', "RO");
                break;
            case '!':
                if (i + 1 < n && src[i + 1] == '=') push2("!=", "RO");
                else push1('!', "LO");
                break;
            case '|':
                if (i + 1 < n && src[i + 1] == '|') push2("||", "LO");
                else { set_err(ERR_UNRECOG); return res; }   // 单独的 | 是错误4
                break;
            case '&':
                if (i + 1 < n && src[i + 1] == '&') push2("&&", "LO");
                else { set_err(ERR_UNRECOG); return res; }   // 单独的 & 是错误4
                break;
            case '+': push1('+', "PLUS");      break;
            case '-': push1('-', "MINUS");     break;
            case '*': push1('*', "TIMES");     break;
            case '/': push1('/', "DIVISION");  break;   // 注释在前面已处理掉
            case ',': push1(',', "COMMA");     break;
            case '(': case ')': case '{': case '}':
                      push1(c,   "BRACE");     break;
            case ';': push1(';', "SEMICOLON"); break;
            default:
                set_err(ERR_UNRECOG); return res;
        }
    }
    return res;
}