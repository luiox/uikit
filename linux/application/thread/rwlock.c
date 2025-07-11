#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

// 1. 初始化读写锁
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

int global_resource = 0;

void clean_handler()
{
    // 4. 销毁读写锁
    pthread_rwlock_destroy(&rwlock);
    exit(0);
}

// 写线程
void* thread1_handler(void* args)
{
    while (1) {
        // 2. 添加写锁
        pthread_rwlock_wrlock(&rwlock);
        // 全局资源++
        global_resource++;
        printf("thread1 write global resource : %d\n", global_resource);
        // 3. 解除写锁
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
}

// 读线程
void* thread2_handler(void* args)
{
    while (1) {
        // 2. 添加读锁
        pthread_rwlock_rdlock(&rwlock);
        // 读取全局资源
        printf("thread2 read global resource : %d\n", global_resource);
        // 3. 解除写锁
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
}

// 读线程
void* thread3_handler(void* args)
{
    while (1) {
        // 2. 添加读锁
        pthread_rwlock_rdlock(&rwlock);
        // 读取全局资源
        printf("thread3 read global resource : %d\n", global_resource);
        // 3. 解除写锁
        pthread_rwlock_unlock(&rwlock);
        sleep(1);
    }
}

int main()
{
    // 注册退出时清理函数
    signal(SIGINT, clean_handler);

    // 创建多线程
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, thread1_handler, NULL);
    pthread_create(&t2, NULL, thread2_handler, NULL);
    pthread_create(&t3, NULL, thread3_handler, NULL);

    pthread_detach(t1);
    pthread_detach(t2);
    pthread_detach(t3);

    while (1) {}

    return 0;
}
