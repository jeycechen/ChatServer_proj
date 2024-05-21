#ifndef REDIS_H
#define REDIS_H

#include<hiredis/hiredis.h>
#include<thread>
#include<string>
#include<functional>
using namespace std;


class redis{
public:
    redis();
    ~redis();

    //连接redis服务器
    bool connect();

    //发布消息
    bool publish(int channel,string message);

    //订阅消息
    bool subscribe(int channel);

    //取消订阅
    bool unsubscribe(int channel);

    //在独立线程中接受订阅通道消息
    void observer_channel_message();

    //初始化向业务层 处理通道消息的回调函数
    void init_notify_handler(function<void(int,string)> fn);



private:
    redisContext *_publish_content;
    redisContext *_subscribe_content;
    function<void(int, string)> _notify_message_handler;
};

#endif