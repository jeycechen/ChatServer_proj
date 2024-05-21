#include<redis.hpp>
#include<iostream>

redis::redis():_subscribe_content(nullptr), _publish_content(nullptr){

}
redis::~redis(){
    if(_subscribe_content != nullptr){
        redisFree(_subscribe_content);
    }
    if(_publish_content != nullptr){
        redisFree(_publish_content);
    }
}

//连接redis服务器
bool redis::connect(){
    _publish_content = redisConnect("127.0.0.1", 6379);
    if(_publish_content == nullptr){
        cerr << "publish_content error" << endl;
        return false;
    }  
    _subscribe_content = redisConnect("127.0.0.1", 6379);  
    if(_subscribe_content == nullptr){
        cerr << "scribe_content error" << endl;
        return false;
    }
    thread t([&](){
        observer_channel_message();
    });

    t.detach();

    cout << "connect redis server susscessful" << endl;
    return true;
}

//发布消息
bool redis::publish(int channel,string message){
    redisReply *response = (redisReply *)redisCommand(_publish_content, "PUBLISH %d %s", channel, message.c_str());
    if(nullptr == response){
        cerr << "redis::publish redisReply error" << endl;
        return false;
    }
    freeReplyObject(response);
    return true;
}

//订阅消息
bool redis::subscribe(int channel){
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_content, "SUBCRIBE %d", channel)){
        cerr << "subcribe command failed" << endl;
        return false;
    }
    int done = 0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_content, &done)){
            cerr << "subscibe command failed" << endl;
            return false;
        }
    }
    return true;
}

//取消订阅
bool redis::unsubscribe(int channel){
    if(REDIS_ERR == redisAppendCommand(this->_subscribe_content, "UNSUBCRIBE %d", channel)){
        cerr << "unsubcribe command failed" << endl;
        return false;
    }
    int done = 0;
    while(!done){
        if(REDIS_ERR == redisBufferWrite(this->_subscribe_content, &done)){
            cerr << "unsubscibe command failed" << endl;
            return false;
        }
    }
    return true;

}

//在独立线程中接受订阅通道消息
void redis::observer_channel_message(){
    redisReply* reply = nullptr;
    while(REDIS_OK == redisGetReply(this->_subscribe_content,(void **) &reply)){
        // 订阅收到的消息是一个三个元素的数组
        if(reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr){
            // 通报 订阅消息
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }

    cerr << ">>>>>>>>>>>>>>>>>>>redis::observer_channel_message exit abnormal <<<<<<<<<<<<<<<<<<<<<" << endl;
}

//初始化向业务层 处理通道消息的回调函数
void redis::init_notify_handler(function<void(int,string)> fn){
    cout << "init redis handler sucess" << endl;
    this->_notify_message_handler = fn;
}