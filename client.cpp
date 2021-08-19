#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        exit(0);
    }
    struct sockaddr_in sockaddr;
    sockaddr.sin_family=AF_INET;
    sockaddr.sin_port=htons(9999);
    inet_pton(AF_INET, "127.0.0.1",&sockaddr.sin_addr.s_addr);
    int ret = connect(fd, (struct sockaddr*)&sockaddr,sizeof(sockaddr));
    if(ret ==-1)
    {
        perror("connect ");
        exit(0);
    }
    while(1)
    {
        char recvBuf[1024];
        //sprintf(recvBuf,"data:%d\n",i++);
        fgets(recvBuf,sizeof(recvBuf),stdin);
        write(fd,recvBuf,sizeof(recvBuf));
        printf("recv buf: %s\n",recvBuf);
        sleep(1); 
    }

    close(fd);
    return 0;

}