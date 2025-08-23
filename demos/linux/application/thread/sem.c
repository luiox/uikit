#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MY_ID 7777

const char* sem_str = "/mysem";

int    shm_id;
int*   p;
sem_t* sem;

void clean_handler(int n)
{
    sem_close(sem);
    sem_unlink(sem_str);
    shmdt(p);
    shmctl(shm_id, IPC_RMID, NULL);
    exit(EXIT_SUCCESS);
}

int main()
{
    signal(SIGINT, clean_handler);

    // 创建共享内存的key
    key_t key = ftok(".", MY_ID);
    if (-1 == key) {
        printf("ftok error:%m\n"), exit(EXIT_FAILURE);
    }

    // 创建父子进程
    pid_t pid = fork();
    if (pid == 0) {
        // 子进程
        // 创建共享内存
        shm_id = shmget(key, 4096, IPC_CREAT | 0666);
        if (-1 == shm_id) {
            printf("创建共享内存 error:%m\n"), exit(EXIT_FAILURE);
        }

        // 挂载共享内存
        p = (int*)shmat(shm_id, NULL, 0);
        if (NULL == p) {
            printf("挂载共享内存 error:%m\n"), exit(EXIT_FAILURE);
        }

        // 创建信号量
        sem = sem_open(sem_str, O_CREAT, 0666, 0);
        if (SEM_FAILED == sem) {
            printf("创建信号量 error:%m\n"), exit(EXIT_FAILURE);
        }

        int i = 0;
        while (1) {
            // 进行p操作，减少数据
            sem_wait(sem);
            printf("*p=%d\n", *p);
            sleep(1);
        }
    }
    else if (pid > 0) {
        // 父进程
        // 创建共享内存
        shm_id = shmget(key, 4096, IPC_CREAT | 0666);
        if (-1 == shm_id) {
            printf("创建共享内存 error:%m\n"), exit(EXIT_FAILURE);
        }

        // 挂载共享内存
        p = (int*)shmat(shm_id, NULL, 0);
        if (NULL == p) {
            printf("挂载共享内存 error:%m\n"), exit(EXIT_FAILURE);
        }

        // 创建信号量
        sem = sem_open(sem_str, O_CREAT, 0666, 0);
        if (SEM_FAILED == sem) {
            printf("创建信号量 error:%m\n"), exit(EXIT_FAILURE);
        }

        int i = 0;
        while (1) {
            // 进行v操作，增加数据
            sem_post(sem);
            *p = i++;
            sleep(1);
        }
    }
    else {
        printf("fork error:%m\n"), exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
