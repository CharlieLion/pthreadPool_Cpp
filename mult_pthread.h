#ifndef mult_pthread
#define mult_pthread
#include<queue>
#include<pthread.h>
using callback = void(*)(void* arg);

template <typename T> 
struct Task
{
    Task<T>(): function(nullptr), arg(nullptr){};
    Task<T>(callback f, T* arg):function(f),arg(arg){};
    callback function;
    T* arg;
};

template <typename T> 
class TaskQueue
{
private:
    /* data */
    pthread_mutex_t mutex;
    
    std::queue<Task<T>> m_TaskQ;
public:
    TaskQueue(/* args */);
    ~TaskQueue();
    void addTask(Task<T> task);
    void addTask(callback f, void* arg);
    Task<T> takeTask();
    inline size_t taskNumber()
    {
        return m_TaskQ.size();
    }

};

#endif