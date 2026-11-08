#pragma once
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>
#include <windows.h>

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
    void LoadPlugins();
    bool HasFunction(const std::string& name) const;
    double CallFunction(const std::string& name, double arg) const;
};