#include "chatserver.hpp"
#include "chatservice.hpp"

#include <functional>
#include <string>
#include "json.hpp"
using namespace std;
using namespace placeholders;
using json = nlohmann::json;

ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg),
      _loop(loop)
{
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1)); // 注册链接回调函数

    // 注册读写事件回调函数
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}

void ChatServer::start()
{
    _server.start();
} // 启动服务


// 上报链接相关信息的回调
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    if(!conn->connected()){
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown(); // 释放资源 fd ，用户断开链接
    }
}

// 上报读写事件相关的回调
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                                Buffer *buffer,
                                Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    json js = json::parse(buf); // 数据的反序列化
    //通过 js["megid"]   获取 业务 handler = 》 conn js time
    //目的 完全解耦网络模块 和 业务模块的代码
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    msgHandler(conn, js, time);
}