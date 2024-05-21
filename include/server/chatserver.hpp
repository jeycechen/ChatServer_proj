#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace std;
using namespace muduo;
using namespace muduo::net;


class ChatServer
{
public:
    ChatServer(EventLoop* loop,
            const InetAddress& listenAddr,
            const string& nameArg);
    void start(); // 启动服务

private:
    // 上报链接相关信息的回调
    void  onConnection(const TcpConnectionPtr&);

    //上报读写事件相关的回调
    void  onMessage(const TcpConnectionPtr&,
                            Buffer*,
                            Timestamp);
    TcpServer _server;
    EventLoop *_loop; // 指向事件循环的指针


};

#endif