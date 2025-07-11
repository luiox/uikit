#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int             ready = 0;
pthread_mutex_t lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;

void* thread1_handler(void* args)
{
    // 加锁
    pthread_mutex_lock(&lock);
    // 修改条件变量
    ready = 1;
    pthread_cond_signal(&cond);
    // 解锁
    pthread_mutex_unlock(&lock);

    // do something
    sleep(10);
    return NULL;
}

int main()
{
    // 创建线程
    pthread_t tid;
    pthread_create(&tid, NULL, thread1_handler, NULL);
    pthread_detach(tid);

    // 加锁
    pthread_mutex_lock(&lock);
    // 只要没有改变，就一直等待
    while (0 == ready) {
        pthread_cond_wait(&cond, &lock);
    }
    // 解锁
    pthread_mutex_unlock(&lock);

    // do something
    sleep(20);

    // pthread_cancel();
    return 0;
}