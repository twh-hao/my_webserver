/***************************
 * 循环数组实现的阻塞队列
 * 线程安全，每个操作前都要加互斥锁，完成后再解锁
 *************************/

#ifndef BLOCK_QUEUE_H_
#define BLOCK_QUEUE_H_

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "locker.h"
using namespace std;

template <class T>
class BlockQueue
{
public:
    BlockQueue(init max_size=1000)
    {
        if (max_size <= 0)
        {
            exit(-1);
        }
        max_size_=max_size;
        array_=new T[max_size];
        size_=0;
        front_=-1;
        back_=-1;
    }
    void Clear()
    {
        mutex_.Lock();
        size_=0;
        front_=-1;
        back_=-1;
        mutex_.UnLock();
    }
    ~BlockQueue()
    {
        mutex_.Lock();
        if (array_ != nullptr)
        {
            delete []array_;
        }
        mutex_.UnLock();
    }
    //判断队列是否满了
    bool IsFull()
    {
        mutex_.Lock();
        if (size_ >= max_size_)
        {
            mutex_.UnLock();
            return true;
        }
        mutex_.UnLock();
        return false;
    }
    //判断队列是否为空
    bool IsEmpty()
    {
        mutex_.Lock();
        if (size_ == 0)
        {
            mutex_.UnLock();
            return true;
        }
        mutex_.UnLock();
        return false;
    }
    //返回队首元素 
    bool Front(T &value)
    {
        mutex_.Lock();
        if (size_ == 0)
        {
            mutex_UnLock();
            return false;
        }
        value=array_[front_];
        mutex_.UnLock();
        return true;
    }
    //返回队尾元素
    bool Back(T &value)
    {
        mutex_.Lock();
        if (size_ == 0)
        {
            mutex_.UnLock();
            return false;
        }
        value=array_[back_];
        mutex_.UnLock();
        return true;
    }
    int Size()
    {
        int temp=0;
        mutex_.Lock();
        temp=size_;
        mutex_.UnLock();
        return temp;
    }

    int MaxSize()
    {
        int temp=0;
        mutex_.Lock();
        temp=max_size_;
        mutex_.UnLock();
        return temp;
    }

    //向队列添加元素，需要将所有使用队列的线程唤醒
    //当有元素push进队列，相当于生产者生产了一个元素
    //若当前没有线程等待条件变量，则唤醒没有意义
    bool Push(const T &item)
    {
        mutex_.Lock();
        if (size_ >= max_size_)
        {
            cond_.BroadCast();
            mutex_.UnLock();
            return false;
        }
        back_=(back_+1)%max_size_;
        array_[back_]=item;

        size_++;
        cond_.BroadCast();
        mutex_.UnLock();
        return true;
    }
    //pop时，如果当前队列没有元素，将会等待条件变量
    bool Pop(T &item)
    {

        mutex_.Lock();
        while (size_ <= 0)
        {
            if (!cond_.Wait(mutex_.Get()))
            {
                mutex_.UnLock();
                return false;
            }
        }

        front_=(ront_+1)%max_size_;
        item= array_[front_];
        size_--;
        mutex_.UnLock();
        return true;
    }
    //pop 增加超时处理
    bool Pop(T &item,int ms_timeout)
    {
        struct timespec t={0,0};
        struct timeval now={0,0};
        gettimeofday(&now,NULL);
        if (size_ <= 0)
        {
            t.tv_sec = now.tv_sec + ms_timeout / 1000;
            t.tv_nsec = (ms_timeout % 1000) * 1000;
            if (!cond_.TimeWait(mutex_.Get(),t))
            {
                mutex_.UnLock();
                return false;
            }
        }
    }
private:
    Locker mutex_;
    Cond cond_;

    T *array_;
    int size_;
    int max_size_;
    int front_;
    int back_;
};


#endif //BLOCK_QUEUE_H_