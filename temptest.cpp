#include<iostream>
#include<pthread.h>
using namespace std;

template<typename T>
struct usa
{
    usa<T>(T ta, T tb):a(ta),b(tb){};
    usa<T>():a(nullptr),b(nullptr){};
    T a,b;
};
int main()
{
    pthread_t tid;
    usa<int>ss(2,5);
    cout<<ss.a<<ss.b<<endl;
    cout<<"threadID: "<<tid<<"  ...."<<endl;
    
}