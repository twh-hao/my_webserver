#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/locker.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T>
class   ThreadPool
{
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    ThreadPool(int actor_model, ConnectionPool *connection_pool, int thread_number = 8, int max_request = 10000);
    ~ThreadPool();
    bool Append(T *request, int state);
    bool Append_p(T *request);
private:

    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void *Worker(void *arg);
    void Run();

    //线程池中的线程数
    int thread_number_;        
    int max_requests_;         //请求队列中允许的最大请求数
    pthread_t *threads_;       //描述线程池的数组，其大小为thread_number_
    std::list<T *> work_queue_; //请求队列
    Locker queue_locker_;       //保护请求队列的互斥锁
    Sem queue_status_;            //是否有任务需要处理
    ConnectionPool *connection_pool_;  //数据库
    int actor_model_;          //模型切换

};

template<typename T>
ThreadPool<T>::ThreadPool(int actor_model, ConnectionPool *connection_pool, int thread_number, int max_requests) : actor_model_(actor_model),thread_number_(thread_number), max_requests_(max_requests), threads_(NULL),connection_pool_(connection_pool)
{
    if (thread_number <= 0 || max_requests <= 0)
    {
        throw std::exception();
    }
    threads_ = new pthread_t[thread_number_];
    if (threads_ == nullptr)
    {
        throw std::exception();
    }

    for (int i = 0; i < thread_number; ++i)
    {
        if (pthread_create(threads_ + i, NULL, Worker, this) != 0)
        {
            delete[] threads_;
            throw std::exception();
        }
        if (pthread_detach(threads_[i]))
        {
            delete[] threads_;
            throw std::exception();
        }
    }
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
    delete[] threads_;
}

template <typename T>
bool ThreadPool<T>::Append(T *request, int state)
{
    queue_locker_.Lock();
    if (work_queue_.size() >= max_requests_)
    {
        queue_locker_.Unlock();
        return false;
    }
    request->state_ = state;
    work_queue_.push_back(request);
    queue_locker_.UnLock();
    queue_status_.Post();
    return true;
}

template <typename T>
bool ThreadPool<T>::Append_p(T *request)
{
    queue_locker_.Lock();
    if (work_queue_.size() >= max_requests_)
    {
        queue_locker_.UnLock();
        return false;
    }
    work_queue_.push_back(request);
    queue_locker_.UnLock();
    queue_status_.Post();
    return true;
}

template <typename T>
void *ThreadPool<T>::Worker(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;
    pool->Run();
    return pool;
}

template <typename T>
void ThreadPool<T>::Run()
{
    while (true)
    {
        queue_status_.Wait();
        queue_Locker_.Lock();
        if (work_queue_.empty())
        {
            queue_locker_.UnLock();
            continue;
        }
        T *request = work_queue_.front();
        work_queue_.pop_front();
        queue_locker_.UnLock();
        if (!request)
            continue;
        if (1 == actor_model_)
        {
            if (0 == request->state_)
            {
                if (request->ReadOnce())
                {
                    request->improv = 1;
                    ConnectionRAII mysqlcon(&request->mysql, connection_pool_);
                    request->process();
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
            else
            {
                if (request->write())
                {
                    request->improv = 1;
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else
        {
            connectionRAII mysqlcon(&request->mysql, m_connPool);
            request->process();
        }
    }
}

#endif //THREAD_POOL_H