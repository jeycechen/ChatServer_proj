#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H


#include "user.hpp"
#include <vector>
using namespace std;

//维护好友信息的操作方法
class FriendModel{
public:
    void insert(int id, int friendid);
    void remove(int id, int friendid);
    vector<User> query(int userid);
};


#endif