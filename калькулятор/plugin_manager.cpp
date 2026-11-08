#include "plugin_manager.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

void PluginManager::LoadPlugins() {
    const std::string pluginDir = "./plugins";
    if (!fs::exists(pluginDir)) {
        fs::create_directory(pluginDir);
        std::cout << "[INFO] Folder './plugins' not found and was created.\n";
        return;
    }

    for (const auto& entry : fs::directory_iterator(pluginDir)) {
        if (entry.path().extension() == ".dll") {
            HMODULE hLib = LoadLibraryA(entry.path().string().c_str());
            if (!hLib) {
                std::cerr << "[WARNING] Failed to load DLL: " << entry.path().filename() << "\n";
                continue;
            }

            UniqueHMODULE smartHandle(hLib);

            auto getName = (GetFuncNamePtr)GetProcAddress(hLib, "GetFunctionName");
            auto exec = (ExecFuncPtr)GetProcAddress(hLib, "Execute");

            if (!getName || !exec) {
                std::cerr << "[WARNING] DLL does not export required functions: " << entry.path().filename() << "\n";
                continue;
            }

            std::string funcName = getName();

            functions_[funcName] = [exec, fileName = entry.path().filename().string()](double val) {
                int errCode = 0;
                double res = exec(val, &errCode);
                if (errCode != 0) {
                    throw std::runtime_error("Error in plugin " + fileName + " (possibly invalid arguments)");
                }
                return res;
                };

            loadedLibs_.push_back(std::move(smartHandle));
            std::cout << "[INFO] Loaded plugin: " << funcName << "\n";
        }
    }
}

bool PluginManager::HasFunction(const std::string& name) const {
    return functions_.find(name) != functions_.end();
}

double PluginManager::CallFunction(const std::string& name, double arg) const {
    return functions_.at(name)(arg);
}