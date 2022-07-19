#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sqlconnectionpool.h"

using namespace std;

ConnectionPool::ConnectionPool()
{
    current_connection_=0;
    free_connection_=0;
}

ConnectionPool* ConnectionPool::GetInstance()
{
    static ConnectionPool connectionPool;
    return &connectionPool;
}

//构造初始化
void ConnectionPool::Init(string url,string user,string pass_word,string database_name, int port, int max_connection, int close_log)
{
    url_=url;
    port_=port;
    user_=user;
    pass_word_=pass_word;
    database_name_=database_name;
    close_log_=close_log;

    for (int i=0;i<max_connection;++i)
    {
        MYSQL *connection=nullptr;
        connection=mysql_init(connection);

        if (connection == nullptr)
        {
            LOG_ERROR("MYSQL ERROR");
            exit(1);
        }

        connection= mysql_real_connect(connection, url.c_str(), user.c_str(), pass_word.c_str(), database_name.c_str(), port, nullptr, 0);
        if (connection == nullptr)
        {
            LOG_ERROR("MYSQL ERROR");
            exit(1);
        }
        connection_list.push_back(connection);
        ++free_connection_;
    }

    reserve=Sem(free_connection_);
    max_connection_=free_connection_;

}