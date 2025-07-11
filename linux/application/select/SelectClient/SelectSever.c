#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define NUM 1024

int serverFd;
int clientFd[NUM];

void signal_handler(int r)
{
    // 关闭客户端socket和服务器socket
    for (int i = 0; i < NUM; i++) {
        close(clientFd[i]);
    }
    close(serverFd);
    exit(0);
}

int main(int argc, char* argv[])
{
    // 设置信号处理，退出程序的时候做清理工作
    signal(2, (__sighandler_t)signal_handler);

    // 创建协议地址族
    struct sockaddr_in addr = {0};
    addr.sin_family         = AF_INET;
    addr.sin_addr.s_addr    = inet_addr("127.0.0.1");
    addr.sin_port           = htons(7777);

    // 绑定
    int r = bind(serverFd, (struct sockaddr*)&addr, sizeof addr);
    if (-1 == r) {
        printf("绑定失败:%m\n");
        close(serverFd);
        exit(-1);
    }

    // 监听
    r = listen(serverFd, 10);
    if (-1 == r) {
        printf("监听失败:%m\n");
        close(serverFd);
        exit(-1);
    }

    // 让客户端的socket默认为-1，这样目的是方便判断是否已经连接了。
    for (int i = 0; i < NUM; i++) {
        clientFd[i] = -1;
    }

    // 开始做select监视事件的准备工作
    // 创建描述符集合
    fd_set fds;
    // 清空描述符集合
    FD_ZERO(&fds);
    // 记录最大描述符的值
    int maxFd = -1;

    struct sockaddr_in cAddr = {0};
    int                len   = sizeof(cAddr);

    int  cfd;
    char buff[1024];
    char temp[1024];
    maxFd = 0;
    maxFd = ((maxFd > serverFd) ? maxFd : serverFd);

    while (1) {
        FD_ZERO(&fds);            // 清空
        FD_SET(serverFd, &fds);   // 把服务器的fd放到fds里去
        // 把客户端的fd放到fds里去
        for (int i = 0; i < NUM; i++) {
            if (-1 != clientFd[i]) {
                FD_SET(clientFd[i], &fds);
            }
        }

        r = select(maxFd + 1, &fds, NULL, NULL, NULL);
        if (-1 == r) {
            printf("Select failed! Error:%m!\n");
            close(serverFd);
            return -1;
        }
        else if (0 == r) {
            printf("Select time out!\n");
            continue;
        }
        else {
            printf("检查是否服务器有反应!\n");
            if (FD_ISSET(serverFd, &fds)) {
                cfd = accept(serverFd, (struct sockaddr*)&cAddr, &len);
                if (-1 == cfd) {
                    printf("客户端连接失败:%m\n");
                }
                else {
                    printf("有客户端连接上服务器了!\n");
                    //保存客户端描述符号
                    for (int i = 0; i < NUM; i++) {
                        if (-1 == clientFd[i]) {
                            clientFd[i] = cfd;
                            // 设置最大描述符
                            maxFd = ((maxFd > cfd) ? maxFd : cfd);
                            printf("maxFd:%d\n", maxFd);
                            break;
                        }
                    }
                }
            }
            printf("检查客户端是否有动静!\n");
            for (int i = 0; i < NUM; i++) {
                if (-1 != clientFd[i] && FD_ISSET(clientFd[i], &fds)) {
                    r = recv(clientFd[i], buff, 1023, 0);
                    if (r > 0) {
                        buff[r] = '\0';
                        printf("%d:%s\n", clientFd[i], buff);

                        // 格式化信息
                        memset(temp, 0, 1024);
                        sprintf(temp, "%d-%s", clientFd[i], buff);
                        // 把信息转发给当前连上服务器的每一个客户端
                        for (int j = 0; j < NUM; j++) {
                            if (-1 != clientFd[j]) {
                                send(clientFd[j], temp, strlen(temp), 0);
                            }
                        }
                    }
                    else {
                        printf("%d客户端断开连接!\n", clientFd[i]);
                        clientFd[i] = -1;
                    }
                }
            }
        }
    }

    return 0;
}
