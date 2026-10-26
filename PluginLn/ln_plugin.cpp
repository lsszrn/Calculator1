#include <cmath>

extern "C" {
    __declspec(dllexport) const char* __cdecl GetFunctionName() {
        return "ln";
    }

    __declspec(dllexport) double __cdecl Execute(double value, int* errorCode) {
        if (value <= 0) {
            *errorCode = 1;
            return 0.0;
        }
        *errorCode = 0;
        return std::log(value);
    }
}