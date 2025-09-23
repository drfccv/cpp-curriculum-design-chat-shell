@echo off
chcp 65001 >nul
echo ==========================================
echo 即时通讯系统 (OICQ) 编译脚本
echo ==========================================

echo.
echo 检查编译环境...

:: 检查是否有 g++ 编译器
g++ --version >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到 g++ 编译器!
    echo 请安装 MinGW 或其他 C++ 编译环境
    pause
    exit /b 1
)

:: 创建目标目录
if not exist obj mkdir obj

echo.
echo 开始编译...

:: 编译 SQLite3 源文件 (使用C编译器)
echo 编译 sqlite3.c...
gcc -std=c99 -Wall -Iinclude/sqlite -c include/sqlite/sqlite3.c -o obj/sqlite3.o

:: 编译所有源文件
echo 编译 database.cpp...
g++ -std=c++11 -Wall -Wextra -Iinclude -Iinclude/sqlite -fexec-charset=UTF-8 -finput-charset=UTF-8 -c src/database.cpp -o obj/database.o

echo 编译 user.cpp...
g++ -std=c++11 -Wall -Wextra -Iinclude -Iinclude/sqlite -fexec-charset=UTF-8 -finput-charset=UTF-8 -c src/user.cpp -o obj/user.o

echo 编译 chat.cpp...
g++ -std=c++11 -Wall -Wextra -Iinclude -Iinclude/sqlite -fexec-charset=UTF-8 -finput-charset=UTF-8 -c src/chat.cpp -o obj/chat.o

echo 编译 ui.cpp...
g++ -std=c++11 -Wall -Wextra -Iinclude -Iinclude/sqlite -fexec-charset=UTF-8 -finput-charset=UTF-8 -c src/ui.cpp -o obj/ui.o

echo 编译 main.cpp...
g++ -std=c++11 -Wall -Wextra -Iinclude -Iinclude/sqlite -fexec-charset=UTF-8 -finput-charset=UTF-8 -c src/main.cpp -o obj/main.o

:: 链接生成可执行文件
echo.
echo 链接生成可执行文件...
g++ obj/sqlite3.o obj/database.o obj/user.o obj/chat.o obj/ui.o obj/main.o -o oicq.exe

if %errorlevel% equ 0 (
    echo.
    echo ==========================================
    echo 编译成功! 
    echo 可执行文件: oicq.exe
    echo ==========================================
    echo.
    echo 运行程序请输入: oicq.exe
    echo 或双击 oicq.exe 文件
) else (
    echo.
    echo ==========================================
    echo 编译失败!
    echo 请检查是否安装了 SQLite3 开发库
    echo ==========================================
)

echo.
pause