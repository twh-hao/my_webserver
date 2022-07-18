#ifndef LOCKER_H_
#define LOCKER_H_

#include <semaphore.h>
#include <exception>
#include <pthread.h>

//信号量
class Sem
{
public:
    Sem()
    {
        //信号量初始化
        if (sem_init(&sem_,0,0) != 0)
        {
            throw std::exception();
        }
    }

    Sem(int num)
    {
        if (sem_init(&sem_,0,num) != 0)
        {
            throw std::exception();
        }
    }

    ~Sem()
    {
        sem_destory(&sem_);
    }

    bool Wait()
    {
        return sem_wait(&sem_) == 0;
    }

    bool Post()
    {
        return sem_post(&sem_) == 0;
    }
private:
    sem_t sem_;
};

//互斥锁
class Locker
{
public:
    Locker()
    {
        if (pthread_mutex_init(&mutex_,NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~Locker()
    {
        pthread_mutex_destory(&mutex_);
    }
    bool Lock()
    {
        return pthread_mutex_lock(&mutex_) == 0;
    }
    bool UnLock()
    {
        return pthread_mutex_unlock(&mutex_) == 0;
    }
    pthread_mutex_t *Get()
    {
        return &mutex_;
    }
private:
    pthread_mutex_t mutex_;
};

//条件变量
class Cond
{
public:
    Cond()
    {
        if (pthread_cond_init(&cond_,NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~Cond()
    {
        pthread_cond_destroy(&cond_);
    }
    bool Wait(pthread_mutex_t &mutex_) //指针改为引用
    {
        int ret=0;
        ret=pthread_cond_wait(&cond_,&mutex_);
        return ret == 0;
    }
    bool TimeWait(pthread_mutex_t *mutex_,struct timespec t)
    {
        int ret=0;
        ret=pthread_cond_timewait(&cond_,mutex,&t);
        return ret == 0;
    }
    bool Signal()
    {
        return pthread_cond_signal(&cond_) == 0;
    }
    bool BroadCast() //一次唤醒所有线程
    {
        return pthread_cond_broadcast(&cond_) == 0;
    }

private:
    pthread_cond_t cond_;
};


#endif //LOCKER_H_