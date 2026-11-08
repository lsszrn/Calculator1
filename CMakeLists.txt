cmake_minimum_required(VERSION 3.15)
project(Calculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(calculator 
    калькулятор/main.cpp
    калькулятор/parser.cpp
    калькулятор/plugin_manager.cpp
)

target_include_directories(calculator PRIVATE калькулятор)

add_library(PluginSin SHARED PluginSin/sin_plugin.cpp)

add_library(PluginLn SHARED PluginLn/ln_plugin.cpp)

if(MSVC)
    set_target_properties(calculator PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
    set_target_properties(PluginSin PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
    set_target_properties(PluginLn PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )
endif()
