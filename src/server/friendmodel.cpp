#include "friendmodel.hpp"
#include "db.h"
#include<iostream>
void FriendModel::insert(int id, int friendid)
{
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO Friend VALUES(%d, %d)", id, friendid);

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // return true;
            return;
        }
        
    }
    return;
}
void FriendModel::remove(int id, int friendid)
{
    return;
}
vector<User> FriendModel::query(int userid)
{
    // cout << "frind query" << endl;
    char sql[1024] = {0};
    sprintf(sql, "SELECT a.id, a.name, a.state from User a inner join Friend b on a.id = b.friendid where b.userid = %d", userid);

    vector<User> ret;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row ;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setID(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                ret.push_back(user);
            }
        }
        else
        {
            LOG_INFO << "Friend INFO query failed, id doesn't exist!";
        }
        mysql_free_result(res);
        
    }
    return ret;
}