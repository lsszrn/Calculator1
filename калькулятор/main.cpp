#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <stdexcept>
#include <cctype>
#include <filesystem>
#include <windows.h>
#include <memory>
#include <locale.h>

namespace fs = std::filesystem;

struct HMODULEDeleter {
    void operator()(HMODULE h) const noexcept {
        if (h) FreeLibrary(h);
    }
};
using UniqueHMODULE = std::unique_ptr<std::remove_pointer_t<HMODULE>, HMODULEDeleter>;

using GetFuncNamePtr = const char* (__cdecl*)();
using ExecFuncPtr = double(__cdecl*)(double, int*);

class PluginManager {
private:
    std::map<std::string, std::function<double(double)>> functions_;
    std::vector<UniqueHMODULE> loadedLibs_;

public:
    void LoadPlugins() {
        const std::string pluginDir = "./plugins";
        if (!fs::exists(pluginDir)) {
            fs::create_directory(pluginDir);
            std::cout << "[INFO] Папка './plugins' не найдена и была создана.\n";
            return;
        }

        for (const auto& entry : fs::directory_iterator(pluginDir)) {
            if (entry.path().extension() == ".dll") {
                HMODULE hLib = LoadLibraryA(entry.path().string().c_str());
                if (!hLib) {
                    std::cerr << "Не удалось загрузить DLL: " << entry.path().filename() << "\n";
                    continue;
                }

                UniqueHMODULE smartHandle(hLib);

                auto getName = (GetFuncNamePtr)GetProcAddress(hLib, "GetFunctionName");
                auto exec = (ExecFuncPtr)GetProcAddress(hLib, "Execute");

                if (!getName || !exec) {
                    std::cerr << "DLL не экспортирует нужные функции: " << entry.path().filename() << "\n";
                    continue;
                }

                std::string funcName = getName();

                functions_[funcName] = [exec, fileName = entry.path().filename().string()](double val) {
                    int errCode = 0;
                    double res = exec(val, &errCode);
                    if (errCode != 0) {
                        throw std::runtime_error("Ошибка в плагине " + fileName +
                            " (возможно, некорректные аргументы)");
                    }
                    return res;
                    };

                loadedLibs_.push_back(std::move(smartHandle));
                std::cout << "[INFO] Загружен плагин: " << funcName << "\n";
            }
        }
    }

    bool HasFunction(const std::string& name) const {
        return functions_.find(name) != functions_.end();
    }

    double CallFunction(const std::string& name, double arg) const {
        return functions_.at(name)(arg);
    }
};

class Parser {
private:
    std::string text_;
    size_t pos_;
    const PluginManager& plugins_;

    char Peek() { SkipSpaces(); return pos_ < text_.length() ? text_[pos_] : '\0'; }
    char Get() { SkipSpaces(); return pos_ < text_.length() ? text_[pos_++] : '\0'; }
    void SkipSpaces() { while (pos_ < text_.length() && std::isspace(text_[pos_])) pos_++; }

    void Expect(char c) {
        if (Get() != c) throw std::runtime_error(std::string("Ожидался символ: ") + c);
    }

    double ParseNumber() {
        SkipSpaces();
        size_t start = pos_;
        bool hasDot = false;
        while (pos_ < text_.length() && (std::isdigit(text_[pos_]) || text_[pos_] == '.')) {
            if (text_[pos_] == '.') {
                if (hasDot) throw std::runtime_error("Некорректное число");
                hasDot = true;
            }
            pos_++;
        }
        if (start == pos_) throw std::runtime_error("Ожидалось число");
        return std::stod(text_.substr(start, pos_ - start));
    }

    std::string ParseIdentifier() {
        SkipSpaces();
        size_t start = pos_;
        while (pos_ < text_.length() && std::isalpha(text_[pos_])) pos_++;
        if (start == pos_) throw std::runtime_error("Ожидалось имя функции");
        return text_.substr(start, pos_ - start);
    }

    double ParseExpr() {
        double result = ParseTerm();
        while (true) {
            char op = Peek();
            if (op == '+') { Get(); result += ParseTerm(); }
            else if (op == '-') { Get(); result -= ParseTerm(); }
            else break;
        }
        return result;
    }

    double ParseTerm() {
        double result = ParsePower();
        while (true) {
            char op = Peek();
            if (op == '*') { Get(); result *= ParsePower(); }
            else if (op == '/') {
                Get();
                double divisor = ParsePower();
                if (divisor == 0.0) throw std::runtime_error("Деление на ноль");
                result /= divisor;
            }
            else break;
        }
        return result;
    }

    double ParsePower() {
        double base = ParseUnary();
        if (Peek() == '^') {
            Get();
            double exp = ParsePower();
            return std::pow(base, exp);
        }
        return base;
    }

    double ParseUnary() {
        if (Peek() == '-') { Get(); return -ParseUnary(); }
        if (Peek() == '+') { Get(); return ParseUnary(); }
        return ParsePrimary();
    }

    double ParsePrimary() {
        char c = Peek();
        if (c == '(') {
            Get();
            double res = ParseExpr();
            Expect(')');
            return res;
        }
        if (std::isalpha(c)) {
            std::string funcName = ParseIdentifier();
            Expect('(');
            double arg = ParseExpr();
            Expect(')');
            if (!plugins_.HasFunction(funcName)) {
                throw std::runtime_error("Неизвестная функция: " + funcName);
            }
            return plugins_.CallFunction(funcName, arg);
        }
        return ParseNumber();
    }

public:
    Parser(const std::string& text, const PluginManager& plugins)
        : text_(text), pos_(0), plugins_(plugins) {
    }

    double Parse() {
        double result = ParseExpr();
        SkipSpaces();
        if (pos_ < text_.length()) throw std::runtime_error("Мусор в конце выражения");
        return result;
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    PluginManager manager;

    try {
        manager.LoadPlugins();
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка при загрузке плагинов: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Калькулятор запущен. Введите выражение (или 'exit' для выхода):\n";

    std::string input;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, input)) break;
        if (input == "exit") break;
        if (input.empty()) continue;

        try {
            Parser parser(input, manager);
            double result = parser.Parse();
            std::cout << "= " << result << "\n";
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] " << e.what() << "\n";
        }
        catch (...) {
            std::cerr << "Неизвестное исключение!\n";
        }
    }

    return 0;
}