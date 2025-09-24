#include "user.h"
#include "database.h"
#include <iostream>

User::User() : id(-1), username(""), password("") {}

User::User(int id, const std::string& username, const std::string& password) 
    : id(id), username(username), password(password) {}

bool User::registerUser(const std::string& username, const std::string& password) {
    Database* db = Database::getInstance();
    
    // 检查用户名是否已存在
    if (db->userExists(username)) {
        std::cout << "用户名已存在！" << std::endl;
        return false;
    }
    
    // 创建新用户
    if (db->createUser(username, password)) {
        std::cout << "用户注册成功！" << std::endl;
        return true;
    } else {
        std::cout << "用户注册失败！" << std::endl;
        return false;
    }
}

bool User::deleteUser(const std::string& username, const std::string& adminPassword) {
    // 验证管理员密码
    if (!verifyAdminPassword(adminPassword)) {
        std::cout << "管理员密码错误！" << std::endl;
        return false;
    }
    
    Database* db = Database::getInstance();
    
    // 检查用户是否存在
    if (!db->userExists(username)) {
        std::cout << "用户不存在！" << std::endl;
        return false;
    }
    
    // 删除用户及相关数据
    if (db->deleteUserData(username)) {
        std::cout << "用户删除成功！" << std::endl;
        return true;
    } else {
        std::cout << "用户删除失败！" << std::endl;
        return false;
    }
}

User* User::login(const std::string& username, const std::string& password) {
    Database* db = Database::getInstance();
    
    if (db->validateUser(username, password)) {
        int userId = db->getUserId(username);
        if (userId != -1) {
            std::cout << "登录成功！欢迎 " << username << std::endl;
            return new User(userId, username, password);
        }
    }
    
    std::cout << "用户名或密码错误！" << std::endl;
    return nullptr;
}

bool User::verifyAdminPassword(const std::string& password) {
    Database* db = Database::getInstance();
    return db->verifySystemPassword(password);
}

bool User::addFriend(const std::string& friendUsername) {
    Database* db = Database::getInstance();
    
    if (friendUsername == this->username) {
        return false;
    }
    
    if (!db->userExists(friendUsername)) {
        return false;
    }
    
    return db->addFriend(this->username, friendUsername);
}

std::vector<std::string> User::getFriends() {
    Database* db = Database::getInstance();
    return db->getFriends(this->username);
}

bool User::createGroup(const std::string& groupName) {
    Database* db = Database::getInstance();
    
    return db->createGroup(groupName, this->username);
}

bool User::joinGroup(const std::string& groupName) {
    Database* db = Database::getInstance();
    
    return db->joinGroup(this->username, groupName);
}

std::vector<std::string> User::getGroups() {
    Database* db = Database::getInstance();
    return db->getUserGroups(this->username);
}

bool User::removeUserFromGroup(const std::string& targetUser, const std::string& groupName, const std::string& adminPassword) {
    Database* db = Database::getInstance();
    
    // 验证权限：必须是群创建者或系统管理员
    bool hasPermission = false;
    
    // 检查是否为群创建者
    if (db->isGroupCreator(this->username, groupName)) {
        hasPermission = true;
    }
    // 或者验证系统管理员密码
    else if (verifyAdminPassword(adminPassword)) {
        hasPermission = true;
    }
    
    if (!hasPermission) {
        return false;
    }
    
    return db->removeFromGroup(targetUser, groupName);
}