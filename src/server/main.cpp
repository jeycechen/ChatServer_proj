#include "chatserver.hpp"
#include <iostream>
#include "chatservice.hpp"
#include <signal.h>
using namespace std;

//处理服务器ctrl+C结束之后， 重置user的状态信息
void resetHandler(int){
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char* argv[]){
    if(argc != 3){
        cerr << "args error" << endl;
        cerr << "usage: ./ChatServer [IP] [PORT]" << endl;
    }
    signal(SIGINT, resetHandler);
    EventLoop loop;
    InetAddress addr(argv[1], atoi(argv[2]));
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}