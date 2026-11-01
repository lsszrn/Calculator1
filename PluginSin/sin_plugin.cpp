#include <cmath>

extern "C" {
    __declspec(dllexport) const char* __cdecl GetFunctionName() {
        return "sin";
    }

    __declspec(dllexport) double __cdecl Execute(double value, int* errorCode) {
        *errorCode = 0;
        return std::sin(value * 3.14159265358979323846 / 180.0);
    }
}