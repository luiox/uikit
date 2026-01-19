#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 定义一个全局的自旋锁
pthread_spinlock_t spinlock;

// 共享资源
int shared_data = 0;

// 线程函数，模拟对共享资源进行操作
void* thread_func(void* arg)
{
    int thread_id = *(int*)arg;

    // 对共享资源进行多次操作
    for (int i = 0; i < 5; ++i) {
        // 尝试获取自旋锁
        pthread_spin_lock(&spinlock);

        // 执行临界区代码
        printf("Thread %d is updating the shared data\n", thread_id);
        shared_data++;
        sleep(1);   // 模拟耗时操作

        // 释放自旋锁
        pthread_spin_unlock(&spinlock);

        // 休眠一段时间，让其他线程有机会执行
        sleep(1);
    }

    printf("Thread %d is done\n", thread_id);
    free(arg);
    return NULL;
}

int main()
{
    // 初始化自旋锁
    pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);

    // 创建两个线程
    pthread_t t1, t2;
    int*      id1 = malloc(sizeof(int));
    int*      id2 = malloc(sizeof(int));
    *id1          = 1;
    *id2          = 2;

    pthread_create(&t1, NULL, thread_func, id1);
    pthread_create(&t2, NULL, thread_func, id2);

    // 等待线程结束
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // 销毁自旋锁
    pthread_spin_destroy(&spinlock);

    printf("Shared data is %d\n", shared_data);
    return 0;
}
