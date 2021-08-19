#include<stdio.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>  //serv_addrin
#include<sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>
#include<string>
#include<iostream>
#include<errno.h>   
#include"threadpool.h"

struct sockrtinfo
{
    sockrtinfo():fd(0),epfd(0){};
    sockrtinfo(int fd, int epfd):fd(fd),epfd(epfd){};
    int fd;
    int epfd;
};

void* acception(void* arg)
{

    sockrtinfo* sf =static_cast<sockrtinfo*>(arg);
    int cfd = accept(sf->fd, NULL, NULL);
    //将文件描述符设置为非阻塞
    //得到相应的额文件描述符属性
    int flag = fcntl(cfd, F_GETFL);
    flag |=O_NONBLOCK;
    fcntl(cfd, F_SETFL, flag);
    //新得到的文件描述符添加到epoll模型，下一轮循环中可以被检测
    //通信文件描述符检测读缓冲区数据的是偶设置为边沿模式
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = cfd;
    int ret = epoll_ctl(sf->epfd, EPOLL_CTL_ADD, cfd, &ev);
    if(ret ==-1)
    {
        perror("epoll_ctl_accept");
        exit(0);
    }
    return nullptr;
}

void* comunicate(void* arg)
{
    sockrtinfo* sf =static_cast<sockrtinfo*>(arg);
    char buf[5];
    memset(&buf,0,sizeof(buf));
    while(1)
    {
        int len= recv(sf->fd, buf, sizeof(buf),0);
        if(len==0)
        {
            std::cout<<"客户断开了连接"<<std::endl;
            epoll_ctl(sf->epfd, EPOLL_CTL_DEL, sf->fd, NULL);
            close(sf->fd);
            break;
        }
        else if(len>0)
        {
            write(STDOUT_FILENO,buf, len);
            send(sf->fd,buf,len, 0);
        }
        else{
            if(errno==EAGAIN) //缓冲区为空
            {   
                std::cout<<"缓冲区为空"<<std::endl;
                break;
            }
            else
            {
                perror("recv");
                break;
            }
        }
    }

}

int main(int argc, const char* argv [])
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd ==-1) 
    {
        perror("socket error");
        exit(1);
    }

    //绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr,0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port= htons(9999);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // the ip reuseaddr
    int opt =1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // bind
    int ret = bind(lfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(ret ==-1)
    {
        perror("bind error");
        exit(1);
    }

    ret = listen(lfd, 64);
    if(ret ==-1)
    {
        perror("listen error");
        exit(1);
    }
    //file decibe
    int epfd = epoll_create(100);
    if(epfd ==-1)
    {
        perror( "epoll_create");
        exit(0);
    }
    // 在epoll中添加节点
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
    if(ret == -1)
    {
        perror("epoll_ctl");
        exit(1);
    }
    struct epoll_event evs[1024];
    int size =sizeof(evs)/sizeof(struct epoll_event);
    while(1)
    {
        int num = epoll_wait(epfd, evs, size, -1);
        printf("=========== num : %d\n", num);
        for(int i=0;i<num;i++)
        {
            int curfd = evs[i].data.fd;
            //判断是否是用于监听
            if(curfd==lfd)
            {
                sockrtinfo(curfd,epfd);
            }
            else
            { 
                //接受信息
                sockrtinfo(curfd,epfd);

            }
        }
    }

    


    
    

}