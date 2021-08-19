#include<pthread.h>
#include<iostream>
#include<string.h>
#include<string>
#include<unistd.h> //sleep
#ifdef threadpool
#define threadpool
#include"threadpool.h"
#endif
using namespace std;

template <typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
    //实例化任务
    do
    {
        m_taskQ = new TaskQueue<T>;
        m_threadIDs = new pthread_t[max];
        if(m_threadIDs==nullptr)
        {
            cout<<"malloc threadIDs failed\n";
        }
        memset(m_threadIDs,0, sizeof(pthread_t)*max);
        m_minNum = min;
        m_maxNum = max;
        m_busyNum = 0 ;
        m_aliveNum = 0;
        m_exitNum = 0;
        if(pthread_mutex_init(&m_lock,NULL)!=0||pthread_cond_init(&m_notEmpty,NULL)!=0)
        {
            cout<<"mutex or condition init fail.."<<endl;
            exit(1);
        }
        pthread_create(&m_managerID,NULL, manager, this);
        for(int i=0;i<min;++i)
        {
            pthread_create(&m_threadIDs[i],NULL, worker, this);
        }
        return;
    }while(0);

    if(m_threadIDs) delete[] m_threadIDs;
    if(m_taskQ) delete m_taskQ;
};

template <typename T>
void* ThreadPool<T>::worker(void* arg)
{
    ThreadPool *pool = static_cast<ThreadPool*>(arg);
    while(true)
    {
        pthread_mutex_lock(&pool->m_lock);
        //判断是否为空--》判断要执行的线程是否为空
        while(pool->m_taskQ->taskNumber()==0&&!pool->m_shutdown)
        {
            cout<<"thread"<<to_string(pthread_self())<<"waiting..."<<endl;
            pthread_cond_wait(&pool->m_notEmpty, &pool->m_lock);   //条件 非空， 拿到mlock (不一定能拿到)
            if(pool->m_exitNum>0)
            {
                pool->m_exitNum--;
                if(pool->m_aliveNum>pool->m_minNum)
                {
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();
                }
            }
        }
        //判断是否关闭
        if(pool->m_shutdown)
        {
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }
        //开始工作
        Task<T> task = pool->m_taskQ->takeTask();
        pool->m_busyNum++;
        pthread_mutex_unlock(&pool->m_lock);
        cout<<"thread "<<to_string(pthread_self())<<" is working...."<<endl;
        task.function(task.arg);
        delete task.arg;
        task.arg =nullptr;

        cout<<"Thread "<<to_string(pthread_self())<<" is finished!..."<<endl;
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);

    }
    return nullptr;
};
template <typename T>
void* ThreadPool<T>::manager(void* arg)
{
    ThreadPool *pool = static_cast<ThreadPool*>(arg);
    while(!pool->m_shutdown)
    {
        sleep(3);
        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_aliveNum;
        int busyNum = pool->m_busyNum;
        int maxNum = pool->m_maxNum;
        pthread_mutex_unlock(&pool->m_lock);
        //create maxnum
        if(queueSize>liveNum&&liveNum<maxNum)
        {
            pthread_mutex_lock(&pool->m_lock);
            int counter = 0;
            for(int i=0;i<maxNum&&counter<NUMBER &&liveNum<maxNum;i++)
            {
                if(pool->m_threadIDs[i]==0)
                {
                    pthread_create(&pool->m_threadIDs[i],NULL,worker,pool);
                    counter++;
                    pool->m_aliveNum++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }
        if(busyNum*2<liveNum&&liveNum>pool->m_minNum)
        {
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);
            for(int i=0;i<NUMBER;i++)
            {
                pthread_cond_signal(&pool->m_notEmpty);
            }

        }
        
    }
    return nullptr;
};
template <typename T>
void ThreadPool<T>::threadExit()
{
    pthread_t pid = pthread_self();
    for(int i=0;i<m_maxNum;i++)
    {
        if(pid==m_threadIDs[i])
        {
            cout<<"destroy the No."<< i << "thread..."<<endl;
            m_threadIDs[i] = 0;
            break;
        }
    }
    pthread_exit(NULL);
};

template <typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
    //pthread_mutex_lock(&m_lock);   
    if(m_shutdown)
    {
        pthread_mutex_unlock(&m_lock);
        return ;
    }
    m_taskQ->addTask(task);
    pthread_cond_signal(&m_notEmpty);
    //pthread_mutex_unlock(&m_lock);
};
template <typename T>
int ThreadPool<T>::getBusyNumber()
{
    pthread_mutex_lock(&m_lock);
    int temp = this->m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return temp;
};
template <typename T>
int ThreadPool<T>::getAliveNumber()
{
    pthread_mutex_lock(&m_lock);
    int temp = this->m_aliveNum;
    pthread_mutex_unlock(&m_lock);
    return temp;
};
template <typename T>
ThreadPool<T>::~ThreadPool()
{
    this->m_shutdown=true;
    pthread_join(m_managerID,NULL); //阻塞回收管理者线程
    for(int i=0;i<m_aliveNum;i++)
    {
        pthread_cond_signal(&m_notEmpty);
    }
    if(m_taskQ)  delete m_taskQ;
    if(m_threadIDs) delete m_threadIDs;
    pthread_cond_destroy(&m_notEmpty);
    pthread_mutex_destroy(&m_lock);    
};
