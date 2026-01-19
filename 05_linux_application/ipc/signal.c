#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

void sighand(int n)
{
    if (2 == n) {
        printf("想杀我，没门，窗户都没有!\n");
    }
    else if (28 == n) {
        printf("----------------------------------------\n");
    }
    else if (14 == n) {
        printf("++++++++++++++++闹钟响了+++++++++++++++++++++\n");
    }
}

int main()
{
    signal(14, sighand);
    signal(SIGWINCH, sighand);
    signal(SIGINT, sighand);   //注册 对 SIGINT 信号的处理为 sighand
    int n = 0;

    sigset_t set, oldSet;
    sigemptyset(&set);
    sigaddset(&set, 2);

    //前10s不加信号屏蔽
    for (int i = 0; i < 10; i++) {
        printf("n:%d pid:%d\n", n++, getpid());
        sleep(1);
    }
    int r;
    if (1 == sigismember(&set, 2)) {
        printf("屏蔽当前信号集!\n");
        r = sigprocmask(SIG_BLOCK, &set, &oldSet);
        if (0 == r)
            printf("设置屏蔽成功!\n");
        else
            printf("设置屏蔽失败:%m\n");
    }
    //屏蔽10s
    for (int i = 0; i < 10; i++) {
        printf("n:%d pid:%d\n", n++, getpid());
        sleep(1);
    }
    if (1 == sigismember(&set, 2)) {
        printf("解除当前信号集屏蔽!\n");
        r = sigprocmask(SIG_UNBLOCK, &set, &oldSet);
        if (0 == r)
            printf("解除屏蔽成功!\n");
        else
            printf("解除屏蔽失败:%m\n");
    }

    while (1) {
        printf("n:%d pid:%d\n", n++, getpid());
        sleep(1);
    }
}
