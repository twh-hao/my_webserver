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
        //信号量初始化
        if (sem_init(&sem_,0,num) != 0)
        {
            throw std::exception();
        }
    }

    ~Sem()
    {
        sem_destroy(&sem_);
    }

    //减小或者锁定当前信号的值
    bool Wait()
    {
        return sem_wait(&sem_) == 0;
    }
    //信号量的值加1
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
        pthread_mutex_destroy(&mutex_);
    }
    //互斥锁上锁
    bool Lock()
    {
        return pthread_mutex_lock(&mutex_) == 0;
    }
    //互斥锁解锁
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
    //阻塞当前线程，等待别的线程唤醒
    bool Wait(pthread_mutex_t &mutex_) //指针改为引用
    {
        int ret=0;
        ret=pthread_cond_wait(&cond_,&mutex_);
        return ret == 0;
    }
    //等待条件变量，设置等待时间
    bool TimeWait(pthread_mutex_t *mutex_,struct timespec t)
    {
        int ret=0;
        ret=pthread_cond_timedwait(&cond_,mutex_,&t);
        return ret == 0;
    }
    //发送一个信号给另外一个正在处于阻塞等待状态的线程,使其脱离阻塞状态,继续执行
    bool Signal()
    {
        return pthread_cond_signal(&cond_) == 0;
    }
    
    //一次唤醒所有线程
    bool BroadCast() 
    {
        return pthread_cond_broadcast(&cond_) == 0;
    }

private:
    pthread_cond_t cond_;
};


#endif //LOCKER_H_