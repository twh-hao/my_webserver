#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"

using namespace std;

class Log
{
public:
    static Log* GetInstance()
    {
        static Log instance;
        return &instance;
    }
    //异步写日志
    static void* FlushLogThread(void *args)
    {
        Log::GetInstance()->AsynchronousWriteLog();
    }

    //可选择的参数有日志文件、日志缓冲区大小、最大行数以及最长日志条队列
    bool Init(const char *file_name, int close_log, int log_buffer_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void WriteLog(int level, const char *format, ...);

    void Flush(void); 

private:
    Log();
    virtual ~Log();

    void* AsynchronousWriteLog()
    {
        string single_log;

        //从阻塞队列中取出一个日志，写入文件
        while (log_queue_->Pop(single_log))
        {
            mutex_.Lock();
            fputs(single_log.c_str(),fp_);
            mutex_.UnLock();
        }
    }



    char directory_name[128]; //路径名
    char log_name[128]; //log文件名
    int split_lines_; //日志最大行数
    int log_buffer_size_; //日志缓冲区大小
    long long count_; //日志行数记录
    int today_; //当前天数
    FILE *fp_; //打开log的文件指针
    char *buffer_;
    BlockQueue<string> *log_queue_; //阻塞队列
    bool is_asynchronous_; //是否同步标志
    Locker mutex_;
    int close_log_; //关闭日志
};

#define LOG_DEBUG(format, ...) if(0 == close_log_) {Log::GetInstance()->WriteLog(0, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_INFO(format, ...) if(0 == close_log_) {Log::GetInstance()->WriteLog(1, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_WARN(format, ...) if(0 == close_log_) {Log::GetInstance()->WriteLog(2, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}
#define LOG_ERROR(format, ...) if(0 == close_log_) {Log::GetInstance()->WriteLog(3, format, ##__VA_ARGS__); Log::GetInstance()->Flush();}

#endif //LOG_H_