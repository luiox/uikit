#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int serverFd;
int clientFd;

void signal_handler()
{
    close(serverFd);
    close(clientFd);
    exit(0);
}

int main(int argc, char* argv[])
{
    signal(2, (__sighandler_t)signal_handler);

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == serverFd) {
        printf("创建socket失败:%m\n");
        exit(-1);
    }

    // 协议地址族
    struct sockaddr_in addr = {0};
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = inet_addr("127.0.0.1");
    addr.sin_port           = htons(7777);

    // 连接服务器
    int r = connect(serverFd, (struct sockaddr*)&addr, sizeof addr);
    if (-1 == r) {
        printf("连接服务器失败:%m\n");
        close(serverFd);
        exit(-1);
    }

    // 通信
    if (0 != fork()) {   // 父进程   循环不断接受用户输入并发送给服务器
        char buff[256];
        while (1) {
            memset(buff, 0, 256);
            printf("client>>");
            scanf("%s", buff);
            send(serverFd, buff, strlen(buff), 0);
        }
    }
    else {   // 子进程 循环不断等待客户端发来数据并显示
        char temp[256];
        while (1) {
            r = recv(serverFd, temp, 255, 0);
            if (r > 0) {
                temp[r] = 0;
                printf("Sever:%s\n", temp);
            }
        }
    }
}
