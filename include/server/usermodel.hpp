#ifndef USERMODEL_H
#define USERMODEL_H

#include "user.hpp"
//    User表的数据操作类
class UserModel{
public:
    // user表的增加方法
    bool insert(User &user);

    //更新用户状态信息
    bool updateState(User &user);

    //查询方法
    User query(int id);

    // 重置用户的状态信息
    void resetState();
};
#endif