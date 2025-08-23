#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#define MY_ID 7777

enum myType
{
    TYPEA = 6666,
    TYPEB,
    TYPEC
};

struct msgbuf
{
    long mtype;
    char mtext[20];
};

int main()
{
    struct msgbuf msg;
    msg.mtype = TYPEA;

    // 1. 创建key
    key_t key = ftok(".", MY_ID);
    if (-1 == key) {
        printf("ftok error:%m\n"), exit(-1);
    }
    printf("ftok %m\n");

    // 2.创建消息队列
    int msgid = msgget(key, IPC_CREAT | 0666);

    int r;
    while (1) {
        memset(msg.mtext, 0, 20);
        // 3. 接收消息
        r = msgrcv(msgid, &msg, sizeof(msg), TYPEA, IPC_NOWAIT);
        printf("r:%d, msg:%s, type:%ld\n", r, msg.mtext, msg.mtype);
        sleep(1);
    }

    // 4. 删除消息队列
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}