#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>

Database* Database::instance = nullptr;

Database::Database() : db(nullptr) {}

Database* Database::getInstance() {
    if (instance == nullptr) {
        instance = new Database();
    }
    return instance;
}

Database::~Database() {
    close();
}

bool Database::initialize() {
    int rc = sqlite3_open("chat.db", &db);
    if (rc) {
        std::cerr << "无法打开数据库: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    // 读取并执行初始化SQL脚本
    std::ifstream sqlFile("database/init.sql");
    if (!sqlFile.is_open()) {
        std::cerr << "无法打开初始化SQL文件" << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << sqlFile.rdbuf();
    std::string sql = buffer.str();
    sqlFile.close();
    
    return executeSQL(sql);
}

void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool Database::executeSQL(const std::string& sql) {
    char* errMsg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL错误: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }
    
    return true;
}

bool Database::createUser(const std::string& username, const std::string& password) {
    std::string sql = "INSERT INTO users (username, password) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "准备SQL语句失败: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::deleteUserData(const std::string& username) {
    // 删除用户相关的所有数据
    std::vector<std::string> queries = {
        "DELETE FROM users WHERE username = ?",
        "DELETE FROM friendships WHERE user1 = ? OR user2 = ?",
        "DELETE FROM group_members WHERE username = ?",
        "DELETE FROM messages WHERE sender = ? OR receiver = ?"
    };
    
    for (const auto& sql : queries) {
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
        if (rc != SQLITE_OK) continue;
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        if (sql.find("user1 = ? OR user2 = ?") != std::string::npos || 
            sql.find("sender = ? OR receiver = ?") != std::string::npos) {
            sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
        }
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    return true;
}

bool Database::validateUser(const std::string& username, const std::string& password) {
    std::string sql = "SELECT password FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        std::string storedPassword = (char*)sqlite3_column_text(stmt, 0);
        sqlite3_finalize(stmt);
        return storedPassword == password;
    }
    
    sqlite3_finalize(stmt);
    return false;
}

bool Database::userExists(const std::string& username) {
    std::string sql = "SELECT COUNT(*) FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    bool exists = false;
    if (rc == SQLITE_ROW) {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

int Database::getUserId(const std::string& username) {
    std::string sql = "SELECT id FROM users WHERE username = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -1;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    int id = -1;
    if (rc == SQLITE_ROW) {
        id = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return id;
}

bool Database::addFriend(const std::string& username, const std::string& friendName) {
    if (!userExists(friendName)) return false;
    
    // 检查是否已经是好友（双向检查）
    std::string checkSql = "SELECT COUNT(*) FROM friendships WHERE (user1 = ? AND user2 = ?) OR (user1 = ? AND user2 = ?)";
    sqlite3_stmt* checkStmt;
    
    int rc = sqlite3_prepare_v2(db, checkSql.c_str(), -1, &checkStmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(checkStmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(checkStmt, 2, friendName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(checkStmt, 3, friendName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(checkStmt, 4, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(checkStmt);
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(checkStmt, 0);
        sqlite3_finalize(checkStmt);
        if (count > 0) {
            return false; // 已经是好友
        }
    } else {
        sqlite3_finalize(checkStmt);
        return false;
    }
    
    // 添加好友关系（双向）
    std::string sql = "INSERT INTO friendships (user1, user2) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    // 添加第一个方向的关系
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, friendName.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) return false;
    
    // 添加反向关系
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, friendName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<std::string> Database::getFriends(const std::string& username) {
    std::vector<std::string> friends;
    std::string sql = "SELECT user2 FROM friendships WHERE user1 = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return friends;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        friends.push_back((char*)sqlite3_column_text(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return friends;
}

bool Database::createGroup(const std::string& groupName, const std::string& creator) {
    std::string sql = "INSERT INTO groups (name, creator) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, groupName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, creator.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    // 创建者自动加入群组
    if (rc == SQLITE_DONE) {
        joinGroup(creator, groupName);
    }
    
    return rc == SQLITE_DONE;
}

bool Database::joinGroup(const std::string& username, const std::string& groupName) {
    std::string sql = "INSERT OR IGNORE INTO group_members (group_name, username) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, groupName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::removeFromGroup(const std::string& username, const std::string& groupName) {
    std::string sql = "DELETE FROM group_members WHERE group_name = ? AND username = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, groupName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool Database::isGroupCreator(const std::string& username, const std::string& groupName) {
    std::string sql = "SELECT creator FROM groups WHERE name = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, groupName.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    bool isCreator = false;
    if (rc == SQLITE_ROW) {
        std::string creator = (char*)sqlite3_column_text(stmt, 0);
        isCreator = (creator == username);
    }
    
    sqlite3_finalize(stmt);
    return isCreator;
}

bool Database::verifySystemPassword(const std::string& password) {
    std::string sql = "SELECT value FROM system_config WHERE key = 'admin_password'";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    rc = sqlite3_step(stmt);
    bool isValid = false;
    if (rc == SQLITE_ROW) {
        std::string storedPassword = (char*)sqlite3_column_text(stmt, 0);
        isValid = (storedPassword == password);
    }
    
    sqlite3_finalize(stmt);
    return isValid;
}

std::vector<std::string> Database::getUserGroups(const std::string& username) {
    std::vector<std::string> groups;
    std::string sql = "SELECT group_name FROM group_members WHERE username = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return groups;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        groups.push_back((char*)sqlite3_column_text(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return groups;
}

std::vector<std::string> Database::getGroupMembers(const std::string& groupName) {
    std::vector<std::string> members;
    std::string sql = "SELECT username FROM group_members WHERE group_name = ?";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return members;
    
    sqlite3_bind_text(stmt, 1, groupName.c_str(), -1, SQLITE_STATIC);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        members.push_back((char*)sqlite3_column_text(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return members;
}

bool Database::saveMessage(const std::string& sender, const std::string& receiver, 
                          const std::string& content, bool isGroup) {
    std::string sql = "INSERT INTO messages (sender, receiver, content, is_group) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, isGroup ? 1 : 0);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

std::vector<Message> Database::getMessages(const std::string& user1, const std::string& user2, 
                                         bool isGroup) {
    std::vector<Message> messages;
    std::string sql;
    
    if (isGroup) {
        sql = "SELECT id, sender, receiver, content, timestamp FROM messages WHERE receiver = ? AND is_group = 1 ORDER BY timestamp ASC LIMIT 50";
    } else {
        sql = "SELECT id, sender, receiver, content, timestamp FROM messages WHERE ((sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?)) AND is_group = 0 ORDER BY timestamp ASC LIMIT 50";
    }
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) return messages;
    
    if (isGroup) {
        sqlite3_bind_text(stmt, 1, user2.c_str(), -1, SQLITE_STATIC); // user2 是群名
    } else {
        sqlite3_bind_text(stmt, 1, user1.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, user2.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, user2.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, user1.c_str(), -1, SQLITE_STATIC);
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Message msg;
        msg.id = sqlite3_column_int(stmt, 0);
        msg.sender = (char*)sqlite3_column_text(stmt, 1);
        msg.receiver = (char*)sqlite3_column_text(stmt, 2);
        msg.content = (char*)sqlite3_column_text(stmt, 3);
        msg.timestamp = (char*)sqlite3_column_text(stmt, 4);
        msg.isGroup = isGroup;
        messages.push_back(msg);
    }
    
    sqlite3_finalize(stmt);
    return messages;
}

std::vector<Database::RecentChat> Database::getRecentChats(const std::string& username) {
    std::vector<RecentChat> recentChats;
    
    // 获取私聊的最近消息 - 修复查询逻辑
    std::string privateSql = R"(
        SELECT 
            CASE 
                WHEN sender = ? THEN receiver 
                ELSE sender 
            END as chat_partner,
            content as last_message,
            MAX(timestamp) as last_time,
            0 as is_group
        FROM messages 
        WHERE (sender = ? OR receiver = ?) AND is_group = 0
        GROUP BY chat_partner
        ORDER BY last_time DESC
    )";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, privateSql.c_str(), -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_STATIC);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            RecentChat chat;
            chat.name = (char*)sqlite3_column_text(stmt, 0);
            chat.lastMessage = (char*)sqlite3_column_text(stmt, 1);
            chat.lastTime = (char*)sqlite3_column_text(stmt, 2);
            chat.isGroup = false;
            recentChats.push_back(chat);
        }
    }
    sqlite3_finalize(stmt);
    
    // 获取群聊的最近消息 - 修复查询逻辑
    std::string groupSql = R"(
        SELECT 
            m.receiver as group_name,
            m.content as last_message,
            MAX(m.timestamp) as last_time,
            1 as is_group
        FROM messages m
        INNER JOIN group_members gm ON m.receiver = gm.group_name
        WHERE gm.username = ? AND m.is_group = 1
        GROUP BY m.receiver
        ORDER BY last_time DESC
    )";
    
    rc = sqlite3_prepare_v2(db, groupSql.c_str(), -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            RecentChat chat;
            chat.name = (char*)sqlite3_column_text(stmt, 0);
            chat.lastMessage = (char*)sqlite3_column_text(stmt, 1);
            chat.lastTime = (char*)sqlite3_column_text(stmt, 2);
            chat.isGroup = true;
            recentChats.push_back(chat);
        }
    }
    sqlite3_finalize(stmt);
    
    // 按时间排序 - 确保最新的在前面
    std::sort(recentChats.begin(), recentChats.end(), 
              [](const RecentChat& a, const RecentChat& b) {
                  return a.lastTime > b.lastTime;
              });
    
    return recentChats;
}