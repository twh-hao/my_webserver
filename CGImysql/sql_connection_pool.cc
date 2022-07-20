#include <mysql/mysql.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.h"

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

//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *ConnectionPool::GetConnection()
{
	MYSQL *connection = NULL;

	if (0 == connection_list.size())
		return NULL;

    //取出连接，信号量减1，为0则等待
	reserve.Wait();
	
	lock.Lock();

	connection = connection_list.front();
	connection_list.pop_front();

	--free_connection_;
	++current_connection_;

	lock.UnLock();
	return connection;
}

//释放当前使用的连接
bool ConnectionPool::ReleaseConnection(MYSQL *connection)
{
	if (NULL == connection)
		return false;

	lock.Lock();

	connection_list.push_back(connection);
	++free_connection_;
	--current_connection_;

	lock.UnLock();

	reserve.Post();
	return true;
}

//销毁数据库连接池
void ConnectionPool::DestroyPool()
{

	lock.Lock();
	if (connection_list.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connection_list.begin(); it != connection_list.end(); ++it)
		{
			MYSQL *connection = *it;
			mysql_close(connection);
		}
		current_connection_ = 0;
		free_connection_ = 0;
		connection_list.clear();
	}

	lock.UnLock();
}

//当前空闲的连接数
int ConnectionPool::GetFreeConnection()
{
	return this->free_connection_;
}

ConnectionPool::~ConnectionPool()
{
	DestroyPool();
}

ConnectionRAII::ConnectionRAII(MYSQL **SQL, ConnectionPool *connection_pool){
	*SQL = connection_pool->GetConnection();
	
	connection_RAII = *SQL;
	pool_RAII = connection_pool;
}

ConnectionRAII::~ConnectionRAII(){
	pool_RAII->ReleaseConnection(connection_RAII);
}