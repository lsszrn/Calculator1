#include "parser.h"
#include "plugin_manager.h"
#include <stdexcept>
#include <cctype>
#include <cmath>

Parser::Parser(const std::string& text, const PluginManager& plugins)
    : text_(text), pos_(0), plugins_(plugins) {
}

char Parser::Peek() {
    SkipSpaces();
    return pos_ < text_.length() ? text_[pos_] : '\0';
}

char Parser::Get() {
    SkipSpaces();
    return pos_ < text_.length() ? text_[pos_++] : '\0';
}

void Parser::SkipSpaces() {
    while (pos_ < text_.length() && std::isspace(text_[pos_])) pos_++;
}

void Parser::Expect(char c) {
    if (Get() != c) throw std::runtime_error(std::string("Expected character: ") + c);
}

double Parser::ParseNumber() {
    SkipSpaces();
    size_t start = pos_;
    bool hasDot = false;
    while (pos_ < text_.length() && (std::isdigit(text_[pos_]) || text_[pos_] == '.')) {
        if (text_[pos_] == '.') {
            if (hasDot) throw std::runtime_error("Invalid number");
            hasDot = true;
        }
        pos_++;
    }
    if (start == pos_) throw std::runtime_error("Expected number");
    return std::stod(text_.substr(start, pos_ - start));
}

std::string Parser::ParseIdentifier() {
    SkipSpaces();
    size_t start = pos_;
    while (pos_ < text_.length() && std::isalpha(text_[pos_])) pos_++;
    if (start == pos_) throw std::runtime_error("Expected function name");
    return text_.substr(start, pos_ - start);
}

double Parser::ParseExpr() {
    double result = ParseTerm();
    while (true) {
        char op = Peek();
        if (op == '+') { Get(); result += ParseTerm(); }
        else if (op == '-') { Get(); result -= ParseTerm(); }
        else break;
    }
    return result;
}

double Parser::ParseTerm() {
    double result = ParsePower();
    while (true) {
        char op = Peek();
        if (op == '*') { Get(); result *= ParsePower(); }
        else if (op == '/') {
            Get();
            double divisor = ParsePower();
            if (divisor == 0.0) throw std::runtime_error("Division by zero");
            result /= divisor;
        }
        else break;
    }
    return result;
}

double Parser::ParsePower() {
    double base = ParseUnary();
    if (Peek() == '^') {
        Get();
        double exp = ParsePower();
        return std::pow(base, exp);
    }
    return base;
}

double Parser::ParseUnary() {
    if (Peek() == '-') { Get(); return -ParseUnary(); }
    if (Peek() == '+') { Get(); return ParseUnary(); }
    return ParsePrimary();
}

double Parser::ParsePrimary() {
    char c = Peek();
    if (c == '(') {
        Get();
        double res = ParseExpr();
        Expect(')');
        return res;
    }
    if (std::isalpha(c)) {
        std::string name = ParseIdentifier();

        if (name == "e" || name == "E") {
            return 2.71828182845904523536;
        }

        Expect('(');
        double arg = ParseExpr();
        Expect(')');
        if (!plugins_.HasFunction(name)) {
            throw std::runtime_error("Unknown function: " + name);
        }
        return plugins_.CallFunction(name, arg);
    }
    return ParseNumber();
}

double Parser::Parse() {
    double result = ParseExpr();
    SkipSpaces();
    if (pos_ < text_.length()) throw std::runtime_error("Unexpected characters at end");
    return result;
}