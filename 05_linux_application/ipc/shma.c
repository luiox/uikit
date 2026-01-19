#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/types.h>

#define MY_ID 7777

int* p = NULL;
int  shmid;

void handler(int n)
{
    if (SIGINT == n) {
        // 4. 卸载共享内存
        shmdt(p);

        // 5. 删除共享内存
        shmctl(shmid, IPC_RMID, NULL);
        exit(0);
    }
}

int main()
{
    printf("pid:%d\n", getpid());
    signal(SIGINT, handler);

    // 1. 创建key
    key_t key = ftok(".", MY_ID);
    if (-1 == key) {
        printf("ftok error:%m\n"), exit(-1);
    }

    printf("ftok %m\n");

    // 2. 创建共享内存
    shmid = shmget(key, 4096, IPC_CREAT | 0666);
    if (-1 == shmid) {
        printf("创建共享内存 error:%m\n"), exit(-1);
    }
    printf("创建共享内存 %m\n");

    // 3. 挂载共享内存
    p = (int*)shmat(shmid, NULL, 0);
    if (NULL == p)
        printf("挂载共享内存 error:%m\n"), exit(-1);
    printf("挂载共享内存 %m\n");

    // 使用
    while (1) {
        printf("%d\n", *p);
        sleep(1);
    }

    return 0;
}
