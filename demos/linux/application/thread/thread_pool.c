#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// 调试用的宏
#define DEBUG_PRINT(msg) printf("File:%s,Line:%d,msg:%s\n",__FILE__, __LINE__, msg)

// 任务参数结构体
typedef struct task_arg{
    void* args;
}task_arg_t;

// 任务函数结构体
typedef struct task_func{
    void* (*task_function)(void*);
}task_func_t;

// 任务结构体
typedef struct task{
    task_func_t task_func;
    task_arg_t arg;
}task_t;

// 任务节点结构体
typedef struct task_node{
    task_t* task;
    struct task_node* next;
}task_node_t;

// 线程节点结构体
typedef struct thread_node{
    pthread_t tid;
    struct thread_node* next;
}thread_node_t;

// 线程池
typedef struct thread_pool
{
    int current_work_queue_num; // 当前工作线程的数量
    int work_queue_min_num; // 最小任务线程数量，就算没有任务也要维持的线程数量
    int work_queue_max_num; // 最大任务线程数量
    pthread_t worker_tid; // 任务线程
    pthread_t supervisor_tid; // 监督线程
    task_node_t* work_head, *work_tail; // 任务队列头和尾
    task_node_t* waitiing_work_head, *waitiing_work_tail; // 等待任务队列头和尾
    thread_node_t* thread_head, *thread_tail; // 线程队列的头和尾
}thread_pool_t;

// 创建一个线程池
thread_pool_t* thread_pool_create(int thread_max_num, int thread_min_num)
{
    thread_pool_t* tp = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    if(NULL == tp){
        DEBUG_PRINT("malloc失败");
        return NULL;
    }
    tp->current_work_queue_num=0;
    tp->work_queue_min_num = thread_min_num;
    tp->work_queue_max_num=thread_max_num;
    tp->work_head = NULL;
    tp->work_tail = NULL;
    tp->waitiing_work_head = NULL;
    tp->waitiing_work_tail =NULL;
    tp->thread_head = NULL;
    tp->thread_tail =NULL;
    return tp;
}

void* thread_pool_worker_thread(void* args)
{
    if(NULL == args){
        DEBUG_PRINT("thread_pool_worker_thread参数为NULL");
        return NULL;
    }
    thread_pool_t* tp = (thread_pool_t*)args;
    // 先初始化工作线程


    while (1) {
    
    }

    return NULL;
}

void* thread_pool_supervisor_thread(void* args)
{
    if(NULL == args){
        DEBUG_PRINT("thread_pool_worker_thread参数为NULL");
        return NULL;
    }
    thread_pool_t* tp = (thread_pool_t*)args;
    // 初始化线程池中的线程，创建线程池中线程的最小数量
    thread_node_t fake_node;
    thread_node_t* p = &fake_node;
    tp->thread_head = p->next;
    for (int i = 0; i < tp->work_queue_min_num; i++) {
        thread_node_t* new_node = (thread_node_t*)malloc(sizeof(thread_node_t));
        if(NULL == new_node){
            DEBUG_PRINT("malloc失败");
            return NULL;
        }
        p->next = new_node;
        p=p->next;
    }
    tp->thread_tail = p;
    
    for(thread_node_t* it = tp->thread_head; it != tp->thread_tail; it=it->next){
        //pthread_create(&it->tid, NULL, it., );
    }
    

    return NULL;
}

// 初始化一个线程池
bool thread_pool_init(thread_pool_t* tp)
{
    if(NULL == tp) {
        DEBUG_PRINT("thread_pool_init参数为NULL");
        return false;
    }
    // 创建任务线程
    pthread_create(&tp->worker_tid, NULL, thread_pool_worker_thread, tp);

    // 创建监督线程
    pthread_create(&tp->supervisor_tid, NULL, thread_pool_supervisor_thread, tp);

    return true;
}

bool thread_pool_destroy(thread_pool_t* tp)
{


    free(tp);
    return true;
}

bool thread_pool_add_task(thread_pool_t* tp, void* (*work)(void*), void* args)
{
    return true;
}

void* work(void* args);

int main()
{
    thread_pool_t* tp = thread_pool_create(4,2);
    thread_pool_init(tp);

    char buf[256];
    for(int i = 0; i < 10; i++){
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "task %d", i);
        thread_pool_add_task(tp, work, buf);
    }

    sleep(300);

    thread_pool_destroy(tp);

    return 0;
}


void* work(void* args)
{
    char* p = args;
    int t = rand()%10;
    printf("args = %s, work timee = %d\n", p, t);
    sleep(t);
    return NULL;
}
