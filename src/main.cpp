#include "ui.h"
#include <iostream>
#include <locale>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

int main() {
    try {
        // 设置UTF-8支持
#ifdef _WIN32
        // Windows平台UTF-8支持
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);

#endif
        
        // 设置本地化 - 使用更安全的方式
        try {
            std::locale::global(std::locale(""));
        } catch (...) {
            // 如果设置失败，使用默认locale
            std::locale::global(std::locale("C"));
        }
        
        UI ui;
        ui.run();
    } catch (const std::exception& e) {
        std::cerr << "程序运行错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}