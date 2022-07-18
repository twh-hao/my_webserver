#ifndef SQL_CONNECTION_POOL_H_
#define SQL_CONNECTION_POOL_H_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "../lock/locker.h"
#include "../log/log.h"

using namespace std;


class ConnectionPool
{
public:
    MYSQL *GetConnection(); //获取数据库连接
    bool ReleaseConnection(MYSQL *connection); //释放连接
    int GetFreeConnection(); //获取连接
    void DestroyPool(); //销毁所有连接

    //单例模式
    static ConnectionPool *GetInstance();

    void init(string url_,string user_,string pass_word_,string database_name_,int port_,int max_connection_,int close_log_);


    string url_; //主机地址
    string port_; //数据库端口号
    strint user_; //登录数据库用户名
    string pass_word_; //登录数据库密码
    string database_name_; //使用数据库名
    int close_log_; //日志开关
private:
    ConnectionPool();
    ~ConnectionPool();

    int max_connection_;
    int current_connection_;
    int free_connection_;

    Locker lock;
    list<MYSQL *> connection_list; //连接池
    Sem reserve;
};

class ConnectionRAII
{
public:
    ConnectionRAII(MYSQL **connection,ConnectionPool *connection_pool);
    ~ConnectionRAII();
private:
    MYSQL *connection_RAII;
    ConnectionPool *pool_RAII;
};


#endif //SQL_CONNECTION_POOL_H_