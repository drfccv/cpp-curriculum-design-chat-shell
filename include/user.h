#ifndef USER_H
#define USER_H

#include <string>
#include <vector>

class User {
public:
    int id;
    std::string username;
    std::string password;
    
    User(int id, const std::string& username, const std::string& password);
    User();
    
    // 用户管理功能
    static bool registerUser(const std::string& username, const std::string& password);
    static bool deleteUser(const std::string& username, const std::string& adminPassword);
    static User* login(const std::string& username, const std::string& password);
    static bool verifyAdminPassword(const std::string& password);
    
    // 好友管理
    bool addFriend(const std::string& friendUsername);
    std::vector<std::string> getFriends();
    
    // 群组管理
    bool createGroup(const std::string& groupName);
    bool joinGroup(const std::string& groupName);
    bool removeUserFromGroup(const std::string& targetUser, const std::string& groupName, const std::string& adminPassword);
    std::vector<std::string> getGroups();
};

#endif