#pragma once
#include <string>

class PluginManager;

class Parser {
private:
    std::string text_;
    size_t pos_;
    const PluginManager& plugins_;

    char Peek();
    char Get();
    void SkipSpaces();
    void Expect(char c);
    double ParseNumber();
    std::string ParseIdentifier();
    double ParseExpr();
    double ParseTerm();
    double ParsePower();
    double ParseUnary();
    double ParsePrimary();

public:
    Parser(const std::string& text, const PluginManager& plugins);
    double Parse();
};