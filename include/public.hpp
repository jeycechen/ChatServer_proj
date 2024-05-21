#ifndef PUBLIC_H
#define PUBLIC_H


// server client 的公共文件
enum EnMsgTpe{
    LOGIN_MSG = 1, // 登陆消息
    LOGIN_MSG_ACK,
    REG_MSG, // 注册消息
    REG_MSG_ACK, //注册响应消息
    ONE_CHAT_MSG, // 一对一聊天消息
    ADD_FRIEND_MSG, // 添加好友信息
};
#endif