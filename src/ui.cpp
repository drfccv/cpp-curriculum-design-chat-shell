#include "ui.h"
#include "database.h"
#include <iostream>
#include <limits>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <ios>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

UI::UI() : currentUser(nullptr), chat(nullptr) {}

UI::~UI() {
    delete currentUser;
    delete chat;
}

void UI::run() {
    // 初始化数据库
    Database* db = Database::getInstance();
    if (!db->initialize()) {
        std::cerr << "数据库初始化失败！" << std::endl;
        return;
    }
    
    std::cout << "========== 欢迎使用即时通讯系统 ==========" << std::endl;
    std::cout << "默认管理员密码: admin123" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    while (true) {
        if (currentUser == nullptr) {
            showMainMenu();
        } else {
            showUserMenu();
        }
    }
}

void UI::showMainMenu() {
    clearScreen();
    std::cout << "\n========== 主菜单 ==========" << std::endl;
    std::cout << "1. 用户注册" << std::endl;
    std::cout << "2. 用户登录" << std::endl;
    std::cout << "3. 删除用户 (需要管理员密码)" << std::endl;
    std::cout << "4. 退出程序" << std::endl;
    std::cout << "===========================" << std::endl;
    
    int choice = getChoice(1, 4);
    
    switch (choice) {
        case 1:
            clearScreen();
            handleRegister();
            break;
        case 2:
            clearScreen();
            handleLogin();
            break;
        case 3:
            clearScreen();
            handleDeleteUser();
            break;
        case 4:
            std::cout << "再见！" << std::endl;
            exit(0);
        default:
            std::cout << "无效选择！" << std::endl;
    }
}

void UI::showUserMenu() {
    clearScreen();
    std::cout << "\n========== 用户菜单 ==========" << std::endl;
    std::cout << "当前用户: " << currentUser->username << std::endl;
    std::cout << "1. 聊天功能" << std::endl;
    std::cout << "2. 添加好友" << std::endl;
    std::cout << "3. 创建群组" << std::endl;
    std::cout << "4. 加入群组" << std::endl;
    std::cout << "5. 移除群成员 (需要权限)" << std::endl;
    std::cout << "6. 退出登录" << std::endl;
    std::cout << "=============================" << std::endl;
    
    int choice = getChoice(1, 6);
    
    switch (choice) {
        case 1:
            clearScreen();
            showChatMenu();
            break;
        case 2:
            clearScreen();
            handleAddFriend();
            break;
        case 3:
            clearScreen();
            handleCreateGroup();
            break;
        case 4:
            clearScreen();
            handleJoinGroup();
            break;
        case 5:
            clearScreen();
            handleRemoveFromGroup();
            break;
        case 6:
            delete currentUser;
            currentUser = nullptr;
            delete chat;
            chat = nullptr;
            std::cout << "已退出登录！" << std::endl;
            break;
        default:
            std::cout << "无效选择！" << std::endl;
    }
}

void UI::showChatMenu() {
    clearScreen();
    std::cout << "\n========== 聊天菜单 ==========" << std::endl;
    std::cout << "1. 最近聊天 (按时间排序)" << std::endl;
    std::cout << "2. 查看好友和群组列表" << std::endl;
    std::cout << "3. 发起私聊" << std::endl;
    std::cout << "4. 发起群聊" << std::endl;
    std::cout << "5. 返回上级菜单" << std::endl;
    std::cout << "=============================" << std::endl;
    
    int choice = getChoice(1, 5);
    
    switch (choice) {
        case 1:
            clearScreen();
            chat->showRecentChats();
            break;
        case 2:
            clearScreen();
            chat->showChatList();
            pauseScreen();
            break;
        case 3:
            clearScreen();
            handlePrivateChat();
            break;
        case 4:
            clearScreen();
            handleGroupChat();
            break;
        case 5:
            return;
        default:
            std::cout << "无效选择！" << std::endl;
    }
}

void UI::handleLogin() {
    std::cout << "========== 用户登录 ==========" << std::endl;
    std::string username = getInput("请输入用户名: ");
    std::string password = getPassword("请输入密码: ");
    
    User* user = User::login(username, password);
    if (user != nullptr) {
        currentUser = user;
        chat = new Chat(username);
        std::cout << "登录成功！" << std::endl;
    } else {
        std::cout << "登录失败！用户名或密码错误。" << std::endl;
        pauseScreen();
    }
}

void UI::handleRegister() {
    std::cout << "========== 用户注册 ==========" << std::endl;
    std::string username = getInput("请输入用户名: ");
    std::string password = getPassword("请输入密码: ");
    
    if (User::registerUser(username, password)) {
        std::cout << "注册成功！" << std::endl;
    } else {
        std::cout << "注册失败！用户名可能已存在。" << std::endl;
    }
    pauseScreen();
}

void UI::handleDeleteUser() {
    std::cout << "========== 删除用户 ==========" << std::endl;
    std::string username = getInput("请输入要删除的用户名: ");
    std::string adminPassword = getPassword("请输入管理员密码: ");
    
    if (User::deleteUser(username, adminPassword)) {
        std::cout << "用户删除成功！" << std::endl;
    } else {
        std::cout << "用户删除失败！请检查用户名和管理员密码。" << std::endl;
    }
    pauseScreen();
}

void UI::handleAddFriend() {
    std::cout << "========== 添加好友 ==========" << std::endl;
    std::string friendName = getInput("请输入要添加的好友用户名: ");
    
    // 检查是否尝试添加自己
    if (friendName == currentUser->username) {
        std::cout << "不能添加自己为好友！" << std::endl;
        pauseScreen();
        return;
    }
    
    // 检查用户是否存在
    Database* db = Database::getInstance();
    if (!db->userExists(friendName)) {
        std::cout << "用户不存在！请确认用户名是否正确。" << std::endl;
        pauseScreen();
        return;
    }
    
    // 尝试添加好友
    if (currentUser->addFriend(friendName)) {
        std::cout << "好友添加成功！" << std::endl;
    } else {
        std::cout << "好友添加失败！该用户已经是您的好友。" << std::endl;
    }
    pauseScreen();
}

void UI::handleCreateGroup() {
    std::cout << "========== 创建群组 ==========" << std::endl;
    std::string groupName = getInput("请输入群组名称: ");
    if (currentUser->createGroup(groupName)) {
        std::cout << "群组创建成功！" << std::endl;
    } else {
        std::cout << "群组创建失败！群组名可能已存在。" << std::endl;
    }
    pauseScreen();
}

void UI::handleJoinGroup() {
    std::cout << "========== 加入群组 ==========" << std::endl;
    std::string groupName = getInput("请输入要加入的群组名称: ");
    if (currentUser->joinGroup(groupName)) {
        std::cout << "加入群组成功！" << std::endl;
    } else {
        std::cout << "加入群组失败！群组可能不存在或已在群中。" << std::endl;
    }
    pauseScreen();
}

void UI::handleRemoveFromGroup() {
    std::cout << "========== 移除群成员 ==========" << std::endl;
    std::string targetUser = getInput("请输入要移除的用户名: ");
    std::string groupName = getInput("请输入群组名称: ");
    std::string adminPassword = getPassword("请输入管理员密码 (如果您是群创建者请随意输入): ");
    
    // 检查用户和群组是否存在
    Database* db = Database::getInstance();
    if (!db->userExists(targetUser)) {
        std::cout << "目标用户不存在！" << std::endl;
        pauseScreen();
        return;
    }
    
    // 检查是否有权限
    if (!db->isGroupCreator(currentUser->username, groupName) && !User::verifyAdminPassword(adminPassword)) {
        std::cout << "权限不足！只有群创建者或系统管理员可以移除群成员！" << std::endl;
        pauseScreen();
        return;
    }
    
    if (currentUser->removeUserFromGroup(targetUser, groupName, adminPassword)) {
        std::cout << "用户 " << targetUser << " 已从群 " << groupName << " 中移除！" << std::endl;
    } else {
        std::cout << "用户移除失败！用户可能不在该群组中。" << std::endl;
    }
    pauseScreen();
}

void UI::handleChatList() {
    chat->showChatList();
    pauseScreen();
}

void UI::handlePrivateChat() {
    std::cout << "========== 发起私聊 ==========" << std::endl;
    std::string friendName = getInput("请输入要聊天的好友用户名: ");
    
    // 检查是否为好友
    Database* db = Database::getInstance();
    std::vector<std::string> friends = db->getFriends(currentUser->username);
    bool isFriend = false;
    for (const auto& f : friends) {
        if (f == friendName) {
            isFriend = true;
            break;
        }
    }
    
    if (!isFriend) {
        std::cout << "该用户不是您的好友！" << std::endl;
        pauseScreen();
        return;
    }
    
    // 直接进入统一的聊天界面
    chat->interactiveChat(friendName, false);
}

void UI::handleGroupChat() {
    std::cout << "========== 发起群聊 ==========" << std::endl;
    std::string groupName = getInput("请输入要进入的群组名称: ");
    
    // 检查是否在群组中
    Database* db = Database::getInstance();
    std::vector<std::string> groups = db->getUserGroups(currentUser->username);
    bool inGroup = false;
    for (const auto& g : groups) {
        if (g == groupName) {
            inGroup = true;
            break;
        }
    }
    
    if (!inGroup) {
        std::cout << "您不在该群组中！" << std::endl;
        pauseScreen();
        return;
    }
    
    // 直接进入统一的聊天界面
    chat->interactiveChat(groupName, true);
}

std::string UI::getInput(const std::string& prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

std::string UI::getPassword(const std::string& prompt) {
    std::cout << prompt;
    std::string password;
    
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                std::cout << "\b \b";
                password.pop_back();
            }
        } else {
            std::cout << '*';
            password += ch;
        }
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    std::getline(std::cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    
    std::cout << std::endl;
    return password;
}

int UI::getChoice(int min, int max) {
    int choice;
    std::cout << "请选择 (" << min << "-" << max << "): ";
    
    while (!(std::cin >> choice) || choice < min || choice > max) {
        std::cout << "无效输入！请输入 " << min << "-" << max << " 之间的数字: ";
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }
    
    std::cin.ignore(); // 清除缓冲区中的换行符
    return choice;
}

void UI::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void UI::pauseScreen() {
    std::cout << "\n按回车键继续...";
    std::cin.get();
}