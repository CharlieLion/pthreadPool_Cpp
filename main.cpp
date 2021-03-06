#include <unistd.h>
#include <iostream>
#include"threadpool.h"
#include"threadpool.cpp"


//测试主文件，用于调用线程池与开启函数通信

void taskFunc(void* arg)
{
    int* num = static_cast<int*>(arg);
    std::cout<<"thread "<<pthread_self()<<" is working..."<<std::endl;
    sleep(1);
}

int main()
{
    ThreadPool<int> pool(2,10);
    for(int i=0;i<100;i++)
    {
        int* num = new int(i+100);
        pool.addTask(Task<int>(taskFunc,num));
    }
    sleep(20);
    return 0;
}
