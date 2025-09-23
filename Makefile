# 即时通讯系统 Makefile

# 编译器设置
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude -Iinclude/sqlite -fexec-charset=UTF-8 -finput-charset=UTF-8
LDFLAGS = 

# 目录设置
SRCDIR = src
INCDIR = include
OBJDIR = obj
DBDIR = database

# 源文件和目标文件
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
SQLITE_OBJ = $(OBJDIR)/sqlite3.o
TARGET = oicq

# 默认目标
all: $(TARGET)

# 创建目标文件目录
$(OBJDIR):
	mkdir -p $(OBJDIR)

# 编译目标文件
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译 SQLite3 源文件 (使用C编译器)
$(SQLITE_OBJ): include/sqlite/sqlite3.c | $(OBJDIR)
	gcc -std=c99 -Wall -Iinclude/sqlite -c $< -o $@

# 链接生成可执行文件
$(TARGET): $(OBJECTS) $(SQLITE_OBJ)
	$(CXX) $(OBJECTS) $(SQLITE_OBJ) -o $(TARGET) $(LDFLAGS)

# 清理编译文件
clean:
	rm -rf $(OBJDIR) $(TARGET) *.db

# 安装 SQLite (Windows)
install-sqlite-windows:
	@echo "请手动下载并安装 SQLite:"
	@echo "1. 访问 https://www.sqlite.org/download.html"
	@echo "2. 下载 sqlite-tools-win32-x86 和 sqlite-dll-win32-x86"
	@echo "3. 将 sqlite3.exe 和 sqlite3.dll 放到系统 PATH 中"

# 安装 SQLite (Linux/macOS)
install-sqlite-linux:
	sudo apt-get update && sudo apt-get install sqlite3 libsqlite3-dev

# 运行程序
run: $(TARGET)
	./$(TARGET)

# 重新编译
rebuild: clean all

# 调试版本
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# 发布版本
release: CXXFLAGS += -O2 -DNDEBUG
release: $(TARGET)

.PHONY: all clean install-sqlite-windows install-sqlite-linux run rebuild debug release