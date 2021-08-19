#include<stdlib.h>
#include<iostream>
#include"mult_pthread.h"
using namespace std;

//#include"mult_pthread.cpp"

template <typename T> 
TaskQueue<T>::TaskQueue()
{
    pthread_mutex_init(&mutex,NULL);
}
template <typename T> 
TaskQueue<T>::~TaskQueue()
{
    pthread_mutex_destroy(&mutex);
}

template <typename T> 
void TaskQueue<T>:: addTask(Task<T> task)
{
    pthread_mutex_lock(&mutex);
    m_TaskQ.push(task);
    pthread_mutex_unlock(&mutex);
}

template <typename T> 
void TaskQueue<T>::addTask(callback f, void* arg)
{
    pthread_mutex_lock(&mutex);
    m_TaskQ.push(Task<T>(f, arg));
    pthread_mutex_unlock(&mutex);
}

template <typename T> 
Task<T> TaskQueue<T>::takeTask()
{
    Task<T> t;
    pthread_mutex_lock(&mutex);
    if(!m_TaskQ.empty())
    {
        t = m_TaskQ.front();
        m_TaskQ.pop();
    }
    pthread_mutex_unlock(&mutex);
    return t;
}


// void func(void* a)
// {
//     int *u = static_cast<int*>(a);
//     cout<<"12345"<<endl;
// }
// int main()
// {
//     int num = 10;
//     Task<int>su(func, &num);
//     cout<<su.arg<<endl;
// }