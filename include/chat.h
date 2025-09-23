#ifndef CHAT_H
#define CHAT_H

#include <string>
#include <vector>
#include "database.h"

class Chat {
private:
    std::string currentUser;
    
public:
    Chat(const std::string& username);
    
    // 聊天功能
    void showChatList();
    void showRecentChats();
    void selectAndEnterChat();
    void interactiveChat(const std::string& target, bool isGroup);
    
    // 消息操作
    bool sendMessage(const std::string& receiver, const std::string& content, bool isGroup = false);
    void showChatHistory(const std::string& target, bool isGroup = false);
    
    // 显示功能
    void displayMessages(const std::vector<Message>& messages);
    void displayRecentChatsList();
    void clearScreen();
    std::string getCurrentTime();
    std::string formatTimeDisplay(const std::string& timestamp);
    std::string formatMessageTime(const std::string& timestamp);
    std::string getLastMessageTime(const std::string& target, bool isGroup);
};

#endif