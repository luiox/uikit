#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

// 1. 初始化信号量和互斥量
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int n = 0;

void* f1(void* args)
{
    for (int i = 0; i < 50000000; i++) {
        // 2. 加锁
        pthread_mutex_lock(&lock);
        // 等待信号量
        pthread_cond_wait(&cond, &lock);
        n++;
        printf("线程一 -------------------------\n");
        // 3. 解锁
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

void* f2(void* args)
{
    for (int i = 0; i < 50000000; i++) {
        // 2. 加锁
        pthread_mutex_lock(&lock);
        // 等待信号量
        pthread_cond_wait(&cond, &lock);
        n++;
        printf("线程二 ---=-----=-------=------=----++++++++++\n");
        // 3. 解锁
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main()
{
    pthread_t p1, p2;

    pthread_mutex_init(&lock, NULL);

    pthread_create(&p1, NULL, f1, NULL);
    pthread_create(&p2, NULL, f2, NULL);

    for (int i = 0; i < 100000000; i++) {
        pthread_cond_signal(&cond);
        sleep(1);
    }
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    // 4. 销毁锁
    pthread_mutex_destroy(&lock);


    printf("n:%d\n", n);
    return 0;
}