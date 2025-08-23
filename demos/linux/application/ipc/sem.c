#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/wait.h>

#define MY_ID 7777

union semun
{
    int              val;   /* 当 cmd 为 SETVAL 时使用 */
    struct semid_ds* buf;   /* 当 cmd 为 IPC_STAT 或 IPC_SET 时使用 */
    unsigned short*  array; /* 当 cmd 为 GETALL 或 SETALL 时使用 */
    struct seminfo*  __buf; /* 当 cmd 为 IPC_INFO 时使用 */
};

void P(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op  = -1;   // P操作
    sem_b.sem_flg = SEM_UNDO;
    semop(sem_id, &sem_b, 1);
}

void V(int sem_id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op  = 1;   // V操作
    sem_b.sem_flg = SEM_UNDO;
    semop(sem_id, &sem_b, 1);
}

void factory(int sem_id)
{
    while (1) {
        // 5. 使用信号量
        P(sem_id);   // 检查是否有空间生产
        printf("生产产品\n");
        V(sem_id);   // 增加产品数量
        sleep(1);    // 模拟生产时间
    }
}

void salesperson(int sem_id)
{
    while (1) {
        // 5. 使用信号量
        V(sem_id);   // 减少产品数量
        printf("销售产品\n");
        sleep(1);    // 每秒销售
        P(sem_id);   // 检查是否有产品可卖
    }
}

int main()
{
    // 1. 创建key
    int key = ftok(".", MY_ID);
    // 2. 创建信号量
    int sem_id = semget(key, 1, 0666 | IPC_CREAT);

    // 3.初始化信号量
    union semun su = {.val = 0};
    semctl(sem_id, 0, SETVAL, su);

    pid_t pid = fork();
    if (pid == 0) {
        factory(sem_id);
    }
    else {
        salesperson(sem_id);
        wait(NULL);
    }

    // 5. 删除信号量
    semctl(sem_id, 0, IPC_RMID, su);
    return 0;
}
