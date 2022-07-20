#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <pthread.h>
#include "log.h"

using namespace std;

Log::Log()
{
    count_=0;
    is_asynchronous_=false;
}

Log::~Log()
{
    if (fp_ != nullptr)
    {
        fclose(fp_);
    }
}

//异步需要设置阻塞队列额长度，同步不需要设置
bool Log::Init(const char *file_name,int close_log,int log_buffer_size, int split_lines, int max_queue_size )
{
    //如果设置了max_queue_size,则设置为异步
    if (max_queue_size >= 1)
    {
        is_asynchronous_=true;
        log_queue_=new BlockQueue<string>(max_queue_size);
        pthread_t tid;
        //FlushLogThread为回调函数，这里表示创建线程异步写日志
        //新创建的线程从FlushLogThread开始执行
        pthread_create(&tid,nullptr,FlushLogThread,nullptr);
    }

    close_log_=close_log;
    log_buffer_size_=log_buffer_size;
    buffer_=new char[log_buffer_size_];
    memset(buffer_,'\0',log_buffer_size_);
    split_lines_=split_lines;

    time_t t=time(nullptr);
    struct tm *sys_tm=localtime(&t);
    struct tm my_tm=*sys_tm;
    //从后往前找到第一个/
    const char *p=strrchr(file_name,'/');
    char log_full_name[256]={0};
    //相当于自定义文件名，若输入的文件名没有/，将时间+文件名作为日志名
    if (p == nullptr)
    {
        snprintf(log_full_name,255,"%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }
    else
    {
        strcpy(log_name, p + 1);
        strncpy(directory_name, file_name, p -file_name + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", directory_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }
}