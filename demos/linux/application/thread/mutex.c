#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int n = 0;

// 定义互斥量
pthread_mutex_t mutex;

void* f1(void* args)
{
    for (int i = 0; i < 500000000; i++) {
        // 2. 加锁
        pthread_mutex_lock(&mutex);
        n++;
        // 3. 解锁
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* f2(void* args)
{
    for (int i = 0; i < 500000000; i++) {
        // 2. 加锁
        pthread_mutex_lock(&mutex);
        n++;
        // 3. 解锁
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main()
{
    pthread_t p1, p2;
    // 1. 初始化互斥量
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&p1, NULL, f1, NULL);
    pthread_create(&p2, NULL, f2, NULL);

    pthread_detach(p1);
    pthread_join(p2, NULL);
    // 4. 销毁锁
    pthread_mutex_destroy(&mutex);


    printf("n:%d\n", n);
    return 0;
}
