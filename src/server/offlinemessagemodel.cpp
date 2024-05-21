#include "offlinemessagemodel.hpp"

#include "db.h"
void OfflineMsgModel::insert(int userid, string msg){
    char sql[1024] = {0};
    sprintf(sql, "INSERT INTO OfflineMessage VALUES(%d, '%s')",
            userid, msg.c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // return true;
            return;
        }
    }

    // return false;
}

void OfflineMsgModel::remove(int userid){
    char sql[1024] = {0};
    sprintf(sql, "DELETE from OfflineMessage where userid = %d",userid);

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // return true;
            return;
        }
    }
}

vector<string> OfflineMsgModel::query(int userid){
    char sql[1024] = {0};
    sprintf(sql, "SELECT message from OfflineMessage where userid = %d", userid);

    vector<string> ret;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row ;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                // 把userid用户的所有离线消息放入vec返回
                ret.push_back(row[0]);
            }
        }
        else
        {
            LOG_INFO << "OFFLINEMSGMODEL query failed, id doesn't exist!";
        }
        mysql_free_result(res);
        
    }

    return ret;
}