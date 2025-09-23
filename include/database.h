#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <vector>

struct Message {
    int id;
    std::string sender;
    std::string receiver;
    std::string content;
    std::string timestamp;
    bool isGroup;
};

class Database {
private:
    sqlite3* db;
    static Database* instance;
    
    Database();
    bool executeSQL(const std::string& sql);
    
public:
    static Database* getInstance();
    ~Database();
    
    bool initialize();
    void close();
    
    // 用户相关操作
    bool createUser(const std::string& username, const std::string& password);
    bool deleteUserData(const std::string& username);
    bool validateUser(const std::string& username, const std::string& password);
    bool userExists(const std::string& username);
    int getUserId(const std::string& username);
    
    // 好友关系操作
    bool addFriend(const std::string& username, const std::string& friendName);
    std::vector<std::string> getFriends(const std::string& username);
    
    // 群组操作
    bool createGroup(const std::string& groupName, const std::string& creator);
    bool joinGroup(const std::string& username, const std::string& groupName);
    bool removeFromGroup(const std::string& username, const std::string& groupName);
    std::vector<std::string> getUserGroups(const std::string& username);
    std::vector<std::string> getGroupMembers(const std::string& groupName);
    bool isGroupCreator(const std::string& username, const std::string& groupName);
    
    // 权限管理
    bool verifySystemPassword(const std::string& password);
    
    // 消息操作
    bool saveMessage(const std::string& sender, const std::string& receiver, 
                    const std::string& content, bool isGroup = false);
    std::vector<Message> getMessages(const std::string& user1, const std::string& user2, 
                                   bool isGroup = false);
    
    // 获取最近聊天列表
    struct RecentChat {
        std::string name;
        std::string lastMessage;
        std::string lastTime;
        bool isGroup;
    };
    std::vector<RecentChat> getRecentChats(const std::string& username);
};

#endif