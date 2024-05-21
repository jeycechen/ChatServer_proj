#ifndef USER_H
#define USER_H

#include <string>
using namespace std;


/// @brief 匹配user 表的 ORM类
class User{
public:
    User(int id = -1, string name = "", string pwd = "", string state ="offline"){
        this->id = id;
        this->name = name;
        this->password = pwd;
        this->state = state;
    }
    
    void setID(int id) {this->id = id;}
    void setName(string name){this->name = name;}
    void setPwd(string pwd){this->password = pwd;}
    void setState(string state) {this->state =state;}

    int getID() {return id;}
    string getName() {return name;}
    string getPwd() {return password;}
    string getState() {return state;}

private:
    int id;
    string name;
    string password;
    string state;
};

#endif