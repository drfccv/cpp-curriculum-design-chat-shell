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
        std::cout << "不能添加自己为好友！" << std::endl;
        return false;
    }
    
    if (!db->userExists(friendUsername)) {
        std::cout << "用户不存在！" << std::endl;
        return false;
    }
    
    if (db->addFriend(this->username, friendUsername)) {
        std::cout << "好友添加成功！" << std::endl;
        return true;
    } else {
        std::cout << "好友添加失败或已经是好友！" << std::endl;
        return false;
    }
}

std::vector<std::string> User::getFriends() {
    Database* db = Database::getInstance();
    return db->getFriends(this->username);
}

bool User::createGroup(const std::string& groupName) {
    Database* db = Database::getInstance();
    
    if (db->createGroup(groupName, this->username)) {
        std::cout << "群组创建成功！" << std::endl;
        return true;
    } else {
        std::cout << "群组创建失败或群名已存在！" << std::endl;
        return false;
    }
}

bool User::joinGroup(const std::string& groupName) {
    Database* db = Database::getInstance();
    
    if (db->joinGroup(this->username, groupName)) {
        std::cout << "加入群组成功！" << std::endl;
        return true;
    } else {
        std::cout << "加入群组失败或已在群中！" << std::endl;
        return false;
    }
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
        std::cout << "权限不足！只有群创建者或系统管理员可以移除群成员！" << std::endl;
        return false;
    }
    
    if (db->removeFromGroup(targetUser, groupName)) {
        std::cout << "用户 " << targetUser << " 已从群 " << groupName << " 中移除！" << std::endl;
        return true;
    } else {
        std::cout << "移除用户失败！" << std::endl;
        return false;
    }
}