#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    // 创建epoll
    int epfd = epoll_create(2);

    // 注册事件
    struct epoll_event ev = {.events = EPOLLIN, .data.fd = 0};

    int r = epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &ev);
    if (-1 == r) {
        perror("注册事件失败");
        exit(EXIT_FAILURE);
    }


    struct epoll_event events[2];
    char               buff[1024];
    while (1) {
        r = epoll_wait(epfd, events, 2, 1000);
        if (r > 0) {
            printf("%d有动静！\n", r);
            if (events[0].data.fd == 0) {
                scanf("%s", buff);
                printf("接受到了:%s\n", buff);
            }
        }
        else if (0 == r) {
            // printf("没有动静!\n");
            continue;
        }
        else {
            printf("出bug了!\n");
        }
    }
    // 关闭fd
    close(epfd);

    return EXIT_SUCCESS;
}
