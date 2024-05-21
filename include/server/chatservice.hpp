#ifndef CHATSERVICE_H
#define CHATSERVICE_H

// 聊天服务器业务类 单例模式
#include<muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <json.hpp>
#include <public.hpp>
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "redis.hpp"
#include <mutex>
using namespace std;
using namespace muduo::net;
using namespace muduo;
using json = nlohmann::json;

// 表示处理消息的 事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json &js, Timestamp time)>;

class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    //处理登陆业务
    void login(const TcpConnectionPtr& conn, json &js, Timestamp time);
    // 注册业务
    void reg(const TcpConnectionPtr& conn, json &js, Timestamp time);

    //  获取消息对应的处理器
    MsgHandler  getHandler(int msgid);
    
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    //添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json &js, Timestamp time);

    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr& conn, json &js, Timestamp time);
    // 服务器异常 业务重置方法
    void reset();

    void handleRedisSubcribeMessage(int channel, string message);
private:
    ChatService();
    // 存储消息id和对应业务的处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;
    

    // 存储在线用户的通信链接
    unordered_map<int, TcpConnectionPtr> _userConnMap; // 需要注意线程安全 // 读写 多线程 

    // 定义互斥锁， 保证 _userConnMap的线程安全
    mutex _connMutex;
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    redis _redis;
};
#endif