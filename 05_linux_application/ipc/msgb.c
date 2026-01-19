#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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

    // 1. 创建key
    key_t key = ftok(".", MY_ID);
    if (-1 == key) {
        printf("ftok error:%m\n"), exit(-1);
    }
    printf("ftok %m\n");

    // 2. 创建消息队列
    int msgid = msgget(key, IPC_CREAT | 0666);

    int r;
    while (1) {
        printf("请输入消息类型:");
        scanf("%ld", &msg.mtype);
        printf("请输入消息:");
        scanf("%s", msg.mtext);
        // 3. 发送消息
        r = msgsnd(msgid, &msg, sizeof msg, IPC_NOWAIT);
        printf("r:%d\n", r);
    }

    // 4. 删除消息队列
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}