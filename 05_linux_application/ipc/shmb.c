#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define MY_ID 7777

int* p = NULL;
int  shmid;

void hander(int n)
{
    if (SIGINT == n) {
        // 4. 卸载共享内存
        shmdt(p);

        // 5. 删除共享内存
        shmctl(shmid, IPC_RMID, NULL);
    }
}
int main()
{
    signal(SIGINT, hander);

    // 1. 创建key
    key_t key = ftok(".", MY_ID);
    if (-1 == key) {
        printf("ftok error:%m\n"), exit(-1);
    }
    printf("ftok %m\n");

    // 2. 创建共享内存
    shmid = shmget(key, 4096, IPC_CREAT);
    if (-1 == shmid) {
        printf("创建共享内存 error:%m\n"), exit(-1);
    }
    printf("创建共享内存 %m\n");

    // 3. 挂载共享内存
    p = (int*)shmat(shmid, NULL, 0);
    if (NULL == p) {
        printf("挂载共享内存 error:%m\n"), exit(-1);
    }
    printf("挂载共享内存 %m\n");

    // 使用
    int n = 0;
    while (1) {
        *p = n++;
        sleep(1);
    }

    return 0;
}
