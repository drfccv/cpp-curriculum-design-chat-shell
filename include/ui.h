#ifndef UI_H
#define UI_H

#include <string>
#include "user.h"
#include "chat.h"

class UI {
private:
    User* currentUser;
    Chat* chat;
    
    void clearScreen();
    void pauseScreen();
    
public:
    UI();
    ~UI();
    
    void run();
    
    // 界面菜单
    void showMainMenu();
    void showUserMenu();
    void showChatMenu();
    
    // 用户操作界面
    void handleLogin();
    void handleRegister();
    void handleDeleteUser();
    
    // 聊天操作界面
    void handleAddFriend();
    void handleCreateGroup();
    void handleJoinGroup();
    void handleRemoveFromGroup();
    void handleChatList();
    void handlePrivateChat();
    void handleGroupChat();
    
    // 输入处理
    std::string getInput(const std::string& prompt);
    std::string getPassword(const std::string& prompt);
    int getChoice(int min, int max);
};

#endif