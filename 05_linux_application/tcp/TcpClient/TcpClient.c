#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>   // socket
#include <netinet/in.h>   // struct sockaddr_in
#include <arpa/inet.h>    // inet_addr
#include <unistd.h>       // close

int severFd;
int clientFd;

void signal_handler(int r)
{
    // 7. 断开连接
    close(severFd);
    close(clientFd);
    exit(0);
}

int main()
{
    signal(2, signal_handler);

    // 1. 创建socket
    severFd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == severFd) {
        printf("创建socket失败\n");
        exit(-1);
    }

    // 2. 确定服务器协议地址族
    struct sockaddr_in addr = {0};
    addr.sin_family         = AF_INET;   // 设置地址族，与socket描述符一致
    addr.sin_addr.s_addr    = inet_addr("127.0.0.1");   // 设置ip为本机回环地址
    addr.sin_port           = htons(9999);              // 端口号

    // 3. 连接服务器
    int ret = connect(severFd, (struct sockaddr*)&addr, sizeof addr);
    if (-1 == ret) {
        printf("连接服务器失败\n");
        exit(-1);
    }

    // 4. 通信
    char buf[256] = {0};
    while (true) {
        memset(buf, 0, sizeof(buf));
        printf("client>>");
        scanf("%s", buf);
        send(severFd, buf, strlen(buf), NULL);
        memset(buf, 0, sizeof(buf));
        ret = recv(severFd, buf, 255, NULL);
        if (ret > 0) {
            buf[ret] = '\0';
            printf("sever>>%s\n", buf);
        }
    }


    return 0;
}
