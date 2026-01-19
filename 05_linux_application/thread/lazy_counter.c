#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

long long total_counter = 0;

// 定义互斥量
pthread_mutex_t mutex;

void* add_handler(void* args)
{
    int counter = 0;
    for (int i = 0; i < 500000000; i++) {
        if (counter == 10000) {
            // 2. 加锁
            pthread_mutex_lock(&mutex);
            total_counter += counter;
            // 3. 解锁
            pthread_mutex_unlock(&mutex);
            counter = 0;
        }
        counter++;
    }
    if (counter != 0) {
        // 2. 加锁
        pthread_mutex_lock(&mutex);
        total_counter += counter;
        // 3. 解锁
        pthread_mutex_unlock(&mutex);
        counter = 0;
    }
    return NULL;
}

int main()
{
    pthread_t p[10];
    // 1. 初始化互斥量
    pthread_mutex_init(&mutex, NULL);

    // 开十个线程
    for (int i = 0; i < 10; i++) {
        pthread_create(&p[i], NULL, add_handler, NULL);
    }
    // 等待所有线程结束
    for (int i = 0; i < 10; i++) {
        pthread_join(p[i], NULL);
    }


    // 4. 销毁锁
    pthread_mutex_destroy(&mutex);


    printf("total_counter:%lld\n", total_counter);
    return 0;
}
