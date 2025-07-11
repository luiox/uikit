#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    char buf[256], param1[256], param2[256];
    scanf("%s %s %s", buf, param1, param2);

    pid_t pid = fork();

    if (pid == 0) {
        // 子进程

        // 这里要注意一定要最后以NULL结束
        execl(buf, param1, param2, NULL);
    }
    else if (pid > 0) {
        // 父进程
        int status;
        wait(&status);   // 等待子进程结束

        // 判断子进程是否因信号结束
        if (WIFSIGNALED(status)) {
            printf("子进程因信号 %d 结束\n", WTERMSIG(status));
        }
        else {
            printf("子进程正常结束\n");
        }
    }

    return 0;
}
