#include "chat.h"
#include "database.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <limits>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

Chat::Chat(const std::string& username) : currentUser(username) {}

void Chat::showChatList() {
    Database* db = Database::getInstance();
    
    std::cout << "\n========== 聊天列表 ==========" << std::endl;
    
    // 显示好友列表
    std::vector<std::string> friends = db->getFriends(currentUser);
    std::cout << "\n好友列表:" << std::endl;
    if (friends.empty()) {
        std::cout << "  暂无好友" << std::endl;
    } else {
        for (size_t i = 0; i < friends.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << friends[i] << std::endl;
        }
    }
    
    // 显示群组列表
    std::vector<std::string> groups = db->getUserGroups(currentUser);
    std::cout << "\n群组列表:" << std::endl;
    if (groups.empty()) {
        std::cout << "  暂未加入任何群组" << std::endl;
    } else {
        for (size_t i = 0; i < groups.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << groups[i] << std::endl;
        }
    }
    
    std::cout << "============================" << std::endl;
}

void Chat::showRecentChats() {
    Database* db = Database::getInstance();
    bool needRefresh = true;  // 标记是否需要刷新显示
    
    while (true) {
        // 需要刷新时显示聊天列表
        if (needRefresh) {
            clearScreen();
            displayRecentChatsList();
            needRefresh = false;
        }
        
        std::cout << "输入序号进入聊天，输入0返回上级菜单: ";
        
        int choice;
        
        // 使用带超时的输入检查
        if (std::cin >> choice) {
            std::cin.ignore(); // 清除缓冲区
            
            if (choice == 0) {
                break;
            } else {
                std::vector<Database::RecentChat> recentChats = db->getRecentChats(currentUser);
                if (choice > 0 && choice <= (int)recentChats.size()) {
                    const auto& selectedChat = recentChats[choice - 1];
                    
                    // 进入选定的聊天
                    interactiveChat(selectedChat.name, selectedChat.isGroup);
                    
                    // 聊天结束后需要刷新显示列表
                    needRefresh = true;
                } else {
                    std::cout << "无效选择!" << std::endl;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    needRefresh = true;
                }
            }
        } else {
            // 输入错误处理
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            std::cout << "无效输入，请输入数字!" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            needRefresh = true;
        }
    }
}

void Chat::selectAndEnterChat() {
    showRecentChats();
}

void Chat::interactiveChat(const std::string& target, bool isGroup) {
    clearScreen();
    std::cout << "\n========== " << (isGroup ? "群聊: " : "私聊: ") << target << " ==========" << std::endl;
    std::cout << "自动刷新中... (每3秒更新)" << std::endl;
    
    // 显示最近的聊天记录
    showChatHistory(target, isGroup);
    
    std::cout << "\n输入消息开始聊天 (输入 'exit' 退出):" << std::endl;
    
    // 记录最后一条消息的时间戳，用于检查是否有新消息
    std::string lastMessageTime = getLastMessageTime(target, isGroup);
    
    // 启动自动刷新线程
    bool shouldExit = false;
    std::thread refreshThread([&]() {
        while (!shouldExit) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            if (!shouldExit) {
                std::string currentLastTime = getLastMessageTime(target, isGroup);
                if (currentLastTime != lastMessageTime && !currentLastTime.empty()) {
                    // 有新消息，刷新显示
                    lastMessageTime = currentLastTime;
                    clearScreen();
                    std::cout << "\n========== " << (isGroup ? "群聊: " : "私聊: ") << target << " ==========" << std::endl;
                    std::cout << "自动刷新中... (每3秒更新)" << std::endl;
                    showChatHistory(target, isGroup);
                    std::cout << "\n" << currentUser << " >> ";
                    std::cout.flush();
                }
            }
        }
    });
    
    std::string message;
    while (true) {
        std::cout << currentUser << " >> ";
        std::getline(std::cin, message);
        
        if (message == "exit") {
            shouldExit = true;
            break;
        } else if (!message.empty()) {
            if (sendMessage(target, message, isGroup)) {
                // 发送后立即显示自己的消息
                std::cout << "[" << getCurrentTime() << "] " << currentUser << ": " << message << std::endl;
                // 更新最后消息时间
                lastMessageTime = getCurrentTime();
            } else {
                std::cout << "消息发送失败" << std::endl;
            }
        }
    }
    
    // 等待刷新线程结束
    if (refreshThread.joinable()) {
        refreshThread.join();
    }
}

std::string Chat::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buffer);
}

void Chat::displayRecentChatsList() {
    Database* db = Database::getInstance();
    std::vector<Database::RecentChat> recentChats = db->getRecentChats(currentUser);
    
    std::cout << "\n========== 最近聊天 (按时间排序) ==========" << std::endl;
    
    if (recentChats.empty()) {
        std::cout << "暂无聊天记录" << std::endl;
        std::cout << "提示: 可以添加好友或创建群聊开始聊天" << std::endl;
    } else {
        std::cout << "序号  类型    名称                最后消息                    时间" << std::endl;
        std::cout << "------------------------------------------------------------------------" << std::endl;
        
        for (size_t i = 0; i < recentChats.size(); ++i) {
            const auto& chat = recentChats[i];
            
            // 截断过长的消息
            std::string shortMsg = chat.lastMessage;
            if (shortMsg.length() > 20) {
                shortMsg = shortMsg.substr(0, 17) + "...";
            }
            
            // 格式化时间显示 - 显示月-日 时:分
            std::string timeStr = formatTimeDisplay(chat.lastTime);
            
            printf("%-4zu  %-6s  %-18s  %-26s  %s\n", 
                   i + 1,
                   chat.isGroup ? "[群聊]" : "[私聊]",
                   chat.name.c_str(),
                   shortMsg.c_str(),
                   timeStr.c_str());
        }
    }
    
    std::cout << "========================================================================" << std::endl;
}

std::string Chat::formatTimeDisplay(const std::string& timestamp) {
    // 输入格式: "YYYY-MM-DD HH:MM:SS"
    if (timestamp.length() >= 19) {
        // 解析消息时间戳
        std::string msgDate = timestamp.substr(0, 10); // YYYY-MM-DD
        std::string todayDate = getCurrentTime().substr(0, 10); // 今天的日期
        
        std::string month = timestamp.substr(5, 2);
        std::string day = timestamp.substr(8, 2);
        std::string time = timestamp.substr(11, 5); // HH:MM
        
        if (msgDate == todayDate) {
            // 今天的消息只显示时间
            return "今天 " + time;
        } else {
            // 其他日期显示月-日 时:分
            return month + "-" + day + " " + time;
        }
    }
    return timestamp;
}

std::string Chat::formatMessageTime(const std::string& timestamp) {
    // 用于聊天记录中的时间显示
    if (timestamp.length() >= 19) {
        // 获取当前日期
        std::string msgDate = timestamp.substr(0, 10); // YYYY-MM-DD
        std::string todayDate = getCurrentTime().substr(0, 10); // 今天的日期
        
        if (msgDate == todayDate) {
            // 今天的消息只显示时间
            return timestamp.substr(11, 8); // HH:MM:SS
        } else {
            // 其他日期显示完整格式
            std::string month = timestamp.substr(5, 2);
            std::string day = timestamp.substr(8, 2);
            std::string time = timestamp.substr(11, 8);
            return month + "-" + day + " " + time;
        }
    }
    return timestamp;
}

std::string Chat::getLastMessageTime(const std::string& target, bool isGroup) {
    Database* db = Database::getInstance();
    std::vector<Message> messages = db->getMessages(currentUser, target, isGroup);
    
    if (!messages.empty()) {
        return messages.back().timestamp;
    }
    return "";
}

void Chat::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

bool Chat::sendMessage(const std::string& receiver, const std::string& content, bool isGroup) {
    Database* db = Database::getInstance();
    return db->saveMessage(currentUser, receiver, content, isGroup);
}

void Chat::showChatHistory(const std::string& target, bool isGroup) {
    Database* db = Database::getInstance();
    std::vector<Message> messages = db->getMessages(currentUser, target, isGroup);
    
    std::cout << "\n聊天记录:" << std::endl;
    if (messages.empty()) {
        std::cout << "暂无聊天记录" << std::endl;
    } else {
        displayMessages(messages);
    }
}

void Chat::displayMessages(const std::vector<Message>& messages) {
    for (const auto& msg : messages) {
        // 使用智能时间格式化
        std::string timeStr = formatMessageTime(msg.timestamp);
        
        if (msg.isGroup) {
            std::cout << "[" << timeStr << "] " << msg.sender << ": " << msg.content << std::endl;
        } else {
            if (msg.sender == currentUser) {
                std::cout << "[" << timeStr << "] 我: " << msg.content << std::endl;
            } else {
                std::cout << "[" << timeStr << "] " << msg.sender << ": " << msg.content << std::endl;
            }
        }
    }
}