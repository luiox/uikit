#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


// 信号处理函数，用于处理SIGCHLD信号
void sigchld_handler(int sig)
{
    int status;

    // 等待子进程结束
    pid_t pid = waitpid(-1, &status, WNOHANG);

    // 如果子进程已经结束
    if (pid != 0) {
        // 处理子进程的退出状态
        if (WIFEXITED(status)) {
            printf("子进程正常终止，退出状态码：%d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            printf("子进程被信号 %d 终止\n", WTERMSIG(status));
        }
    }
}

int main()
{
    // 注册SIGCHLD信号处理函数
    signal(SIGCHLD, sigchld_handler);

    // 创建各种进程
    // ...

    return 0;
}
