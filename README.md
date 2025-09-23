# C++ 即时通讯系统 (OICQ)

一个基于 C++ 和 SQLite 的单机即时通讯系统，支持用户管理、好友聊天、群组聊天等功能。

## 🌟 项目特色

- **完整的用户系统**：注册、登录、删除用户
- **智能聊天列表**：按时间排序显示最近聊天，支持自动刷新
- **多种聊天方式**：私聊、群聊功能完备
- **权限管理**：管理员密码保护的用户删除和群管理
- **跨平台支持**：Windows 和 Linux 兼容
- **UTF-8 支持**：完整的中文显示支持

## 📋 功能列表

### 用户管理
- ✅ 用户注册（用户名、密码）
- ✅ 用户登录
- ✅ 用户删除（需要管理员密码）
- ✅ 好友添加与管理

### 聊天功能
- ✅ 最近聊天列表（按时间排序）
- ✅ 私聊功能
- ✅ 群聊功能
- ✅ 聊天记录查看
- ✅ 实时消息刷新
- ✅ 智能时间显示（今天/历史日期）

### 群组管理
- ✅ 创建群组
- ✅ 加入群组
- ✅ 移除群成员（需要权限验证）

### 界面特性
- ✅ 清晰的菜单导航
- ✅ 自动清屏功能
- ✅ 用户友好的操作提示
- ✅ 错误处理和输入验证

## 🚀 快速开始

### 环境要求

- **编译器**：支持 C++11 的编译器
  - Windows: MinGW-w64 或 Visual Studio
  - Linux: GCC 4.8+ 或 Clang 3.3+
- **数据库**：内置 SQLite（无需额外安装）

### 编译方法

#### Windows (MinGW)
```bash
# 使用提供的批处理脚本
./build.bat

# 或手动编译
g++ -std=c++11 -I./include -I./include/sqlite -O2 -DUTF8_SUPPORT -finput-charset=UTF-8 -fexec-charset=UTF-8 -c include/sqlite/sqlite3.c -o sqlite3.o
g++ -std=c++11 -I./include -I./include/sqlite -O2 -DUTF8_SUPPORT -finput-charset=UTF-8 -fexec-charset=UTF-8 -c src/*.cpp
g++ -o oicq.exe *.o
```

#### Linux
```bash
# 使用提供的脚本
chmod +x build.sh
./build.sh

# 或使用 Makefile
make
```

### 运行程序

```bash
# Windows
./oicq.exe

# Linux
./oicq
```

## 📁 项目结构

```
oicq/
├── src/                    # 源代码文件
│   ├── main.cpp           # 程序入口
│   ├── ui.cpp             # 用户界面实现
│   ├── user.cpp           # 用户管理功能
│   ├── chat.cpp           # 聊天功能实现
│   └── database.cpp       # 数据库操作
├── include/               # 头文件
│   ├── ui.h
│   ├── user.h
│   ├── chat.h
│   ├── database.h
│   └── sqlite/            # SQLite 源码
│       └── sqlite3.c
├── sql/                   # 数据库初始化脚本
│   └── init.sql
├── build.bat              # Windows 编译脚本
├── build.sh               # Linux 编译脚本
├── Makefile               # Make 构建文件
└── README.md              # 项目文档
```

## 🎯 使用指南

### 首次使用

1. **启动程序**：运行 `oicq.exe`
2. **注册用户**：选择"用户注册"创建账户
3. **登录系统**：使用注册的用户名和密码登录

### 基本操作

#### 添加好友
1. 登录后选择"添加好友"
2. 输入好友的用户名
3. 系统会验证用户是否存在并添加好友关系

#### 创建群组
1. 选择"创建群组"
2. 输入群组名称
3. 创建成功后自动成为群主

#### 开始聊天
1. 选择"聊天功能" → "最近聊天"
2. 选择要聊天的好友或群组
3. 输入消息，按回车发送
4. 输入 `exit` 退出当前聊天

### 管理功能

- **删除用户**：需要管理员密码（默认：admin123）
- **移除群成员**：群创建者或管理员权限

## 🔧 技术特性

### 数据库设计
- **users**: 用户信息表
- **friendships**: 好友关系表
- **groups**: 群组信息表  
- **group_members**: 群成员关系表
- **messages**: 消息记录表
- **system_config**: 系统配置表

### 安全特性
- 密码安全输入（隐藏显示）
- 管理员权限验证
- 输入验证和错误处理
- SQL 注入防护（参数化查询）

### 性能优化
- 消息记录限制（最近50条）
- 智能时间显示减少计算
- 内存管理优化
- 数据库连接复用

## 🛠️ 开发说明

### 代码架构

采用模块化设计，主要包含以下模块：

- **UI模块** (`ui.h/cpp`): 用户界面和菜单系统
- **User模块** (`user.h/cpp`): 用户管理和认证
- **Chat模块** (`chat.h/cpp`): 聊天功能和消息处理
- **Database模块** (`database.h/cpp`): 数据库操作和数据持久化

### 编译配置

项目支持多种编译方式：
- **批处理脚本**: 适用于 Windows 快速编译
- **Shell脚本**: 适用于 Linux 环境
- **Makefile**: 跨平台 Make 构建系统

### UTF-8支持

项目完整支持UTF-8编码：
- 源码采用UTF-8编码
- 编译器配置输入输出字符集
- 控制台UTF-8显示设置

## 🤝 贡献指南

欢迎提交 Issues 和 Pull Requests！

### 开发环境设置
1. Fork 项目到个人仓库
2. 克隆到本地：`git clone <your-fork-url>`
3. 创建功能分支：`git checkout -b feature-name`
4. 提交更改：`git commit -m "Add feature"`
5. 推送分支：`git push origin feature-name`
6. 创建 Pull Request

### 代码规范
- 使用4空格缩进
- 函数和变量使用驼峰命名法
- 添加适当的注释
- 遵循现有代码风格

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 📞 联系方式

- **开发者**: drfccv
- **项目地址**: https://github.com/drfccv/cpp-curriculum-design-chat-shell
- **问题报告**: https://github.com/drfccv/cpp-curriculum-design-chat-shell/issues


---

**注意**: 默认管理员密码为 `admin123`

### Windows 环境
1. 确保安装了 MinGW 或 GCC 编译器
2. 编译程序：
   ```bash
   .\build.bat
   ```
3. 运行程序：
   ```bash
   .\oicq.exe
   ```

### Linux/macOS 环境
1. 确保安装了 GCC 编译器：
   ```bash
   # Ubuntu/Debian
   sudo apt-get install gcc g++
   
   # macOS
   xcode-select --install
   ```

2. 编译程序：
   ```bash
   chmod +x build.sh
   ./build.sh
   ```

3. 运行程序：
   ```bash
   ./oicq
   ```

### 使用 Makefile 编译
```bash
make
make run
```

## 使用说明

### 默认配置
- 系统管理员密码：`admin123`
- 数据库文件：`chat.db`（自动创建）

### 主要操作流程

1. **用户注册/登录**
   - 首次使用需要注册用户账号
   - 使用用户名和密码登录系统

2. **添加好友**
   - 通过用户名添加其他用户为好友
   - 只有互为好友才能进行私聊

3. **创建/加入群组**
   - 任何用户都可以创建新群组
   - 通过群组名称加入已存在的群组

4. **聊天交流**
   - 查看聊天列表（好友和群组）
   - 进入私聊或群聊界面
   - 发送和接收消息

5. **权限管理**
   - 群组创建者可以移除群成员
   - 系统管理员可以删除用户和管理群组

## 数据库设计

### 主要表结构
- `users`: 用户信息表
- `friendships`: 好友关系表
- `groups`: 群组信息表
- `group_members`: 群组成员表
- `messages`: 消息记录表
- `system_config`: 系统配置表

## 编译选项

```bash
make debug      # 调试版本
make release    # 发布版本
make clean      # 清理编译文件
make rebuild    # 重新编译
```

## 注意事项

1. 程序运行需要在项目根目录下，确保能找到 `database/init.sql` 文件
2. SQLite数据库文件会在首次运行时自动创建
3. 密码输入时会隐藏显示（Windows和Linux环境）
4. 聊天时输入 `exit` 可以退出当前聊天会话

## 扩展功能建议

- 消息加密存储
- 文件传输功能
- 在线状态显示
- 消息推送通知
- 图形化界面
- 网络通信支持（客户端-服务器架构）