#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <string>
#include <vector>

using namespace std;
using namespace muduo;

ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}

// 注册消息及对应的业务处理方法 // 初始化列表
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});

    if(_redis.connect()){
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubcribeMessage, this, _1, _2));
    }
}

void ChatService::handleRedisSubcribeMessage(int channel, string message){

    //消息经过 redis消息中间件 被转发到了 本机上，本机上有这个id 的连接信息， 直接转发就好了
    // json js = json::parse(message.c_str());
    lock_guard<mutex> lock(_connMutex); // 防止转发的过程中下线了
    auto it = _userConnMap.find(channel);
    if(it != _userConnMap.end()){
        it->second->send(message);
        return;
    }

    //
    _offlineMsgModel.insert(channel, message);

}
void ChatService::reset(){
    // online 用户的状态改成 offline
    _userModel.resetState();
}
MsgHandler ChatService::getHandler(int msgid)
{
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())
    {
        // 返回一个默认的处理器， 空操作；
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time)
        {
            LOG_ERROR << "msgid: " << msgid << " not define, cann't find handler!";
        };
    }

    return _msgHandlerMap[msgid];
}

// 处理登陆业务 ORM object relation mapping 业务层操作的的都是对象  业务模块和数据模块解耦合
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"].get<int>();
    string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getID() != -1 && user.getPwd() == pwd)
    {

        if (user.getState() == "online")
        { // 已经在线，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;

            response["errno"] = 2;
            response["errmsg"] = "this id has logined, re-login is formidable!";
            conn->send(response.dump());
        }
        else
        {
            // 登陆成功 记录用户连接信息
            {
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
            }
            // id 用户登录成功之后，向redis订阅channel(id)
            _redis.subscribe(id);

            // 登陆成功，更新用户状态信息，state offline -> online
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["id"] = user.getID();
            response["errno"] = 0;
            response["name"] = user.getName();
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty()){
                response["offlinemsg"] = vec;

                // 读取完这个用户的离线消息之后， 删除该用户的所有离线消息 防止重复发送；
                _offlineMsgModel.remove(id);
            }
            // 查询 用户 是否有别人发给他的离线消息


            //查询用户好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty()){
                LOG_INFO << "return frinend info" ;
                vector<string> vec2;
                for(User _u:userVec){
                    json js;
                    js["id"] = _u.getID();
                    js["name"] = _u.getName();
                    js["state"] = _u.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;

            }
            else{
                LOG_INFO << "empty friend list" ;
            }
            conn->send(response.dump());

            
        }
    }
    else
    {
        // 失败
        // 用户不存在，或者密码错误，登陆失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;

        response["errno"] = 1;
        response["errmsg"] = "login failed, wrong id or password";
        conn->send(response.dump());
    }
}


// 注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    // name pass wd;
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);

    bool _ok = _userModel.insert(user);

    if (_ok)
    {
        // ok
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["id"] = user.getID();

        response["errno"] = 0;
        conn->send(response.dump());
    }
    else
    {
        // false //
        json response;
        response["msgid"] = REG_MSG_ACK;

        response["errno"] = 1;
        response["errmsg"] = "reg failed, try later please";
        conn->send(response.dump());
    }
}
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        
        for (auto it = _userConnMap.begin(); it != _userConnMap.end(); it++)
        {
            if (it->second == conn)
            {
                // 从map删除用户的链接信息
                user.setID(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 更新用户的状态信息
    if(user.getID() != -1){
        user.setState("offline");
        _userModel.updateState(user);
    }
    
}

void ChatService::oneChat(const TcpConnectionPtr& conn, json &js, Timestamp time){
    int toid = js["to"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end()){ 
            // to ID在线 需要 转发
            it->second->send(js.dump()); // 服务器主动推送消息给 toid 用户 相当于消息中转
            return;
        }
        
    }
    // 本地没有保存连接信息，查询是否在线 
    // 集群服务器需要修改 ,在线 但是在其他集群服务器上
    User user = _userModel.query(toid);
    if(user.getState() == "online"){
        _redis.publish(toid, js.dump());
        return ;
    }

    //不在线 ，没找到 ，存储离线消息 
    _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr& conn, json &js, Timestamp time){
    int id = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();


    _friendModel.insert(id, friendid);
}