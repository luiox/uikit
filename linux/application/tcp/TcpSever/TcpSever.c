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

    // 3. 绑定
    // 这里把struct sockaddr_in*转换为struct sockaddr*的原因
    // 两者首先都是uint16_t，其次，两者的区别仅仅是因为struct
    // sockaddr定义的时候把端口和地址混在一起定义了。 通过先定义struct
    // sockaddr_in，赋值完以后再转换为truct sockaddr可以避免去直接初始化truct sockaddr。
    // 通过引入一个truct sockaddr_in，来解决truct sockaddr定义不够好的问题。
    int ret = bind(severFd, (const struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        printf("绑定socket失败\n");
        exit(-1);
    }

    // 4. 监听
    ret = listen(severFd, 10);
    if (ret == -1) {
        printf("监听socket失败\n");
        exit(-1);
    }

    // 5. 接受客户端连接
    clientFd = accept(severFd, NULL, NULL);
    if (clientFd == -1) {
        printf("接受客户端连接失败\n");
        exit(-1);
    }

    // 6. 通信
    char buf[256] = {0};
    while (true) {
        memset(buf, 0, sizeof(buf));
        ret = recv(clientFd, buf, 255, NULL);
        if (ret > 0) {
            buf[ret] = '\0';
            printf("client>>%s\n", buf);
            memset(buf, 0, sizeof(buf));
            printf("sever>>");
            scanf("%s", buf);
            send(clientFd, buf, strlen(buf), NULL);
        }
    }


    return 0;
}
