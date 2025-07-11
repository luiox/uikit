#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    // 1. 创建两个int的数组存描述符
    int pipefd[2];

    // 2. 把文件描述符转换为管道
    if (pipe(pipefd) == -1) {
        perror("创建管道失败");
    }

    // 3. 创建子进程
    pid_t pid = fork();

    // 4. 使用管道进行通信
    if (pid == 0) {
        // 子进程
        char buf[256];
        while (1) {
            // 从管道读取数据，并显示
            read(pipefd[0], buf, sizeof(buf));
            printf("子进程从管道读取到：%s\n", buf);
        }
    }
    else if (pid > 0) {
        // 父进程
        char buf[256];
        while (1) {
            // 从键盘读入数据，并通过管道发送到子进程
            scanf("%s", buf);
            write(pipefd[1], buf, sizeof(buf));
            printf("父进程从键盘输入：%s\n", buf);
        }
    }
    else {
        perror("创建进程失败");
    }

    // 5. 关闭描述符
    close(pipefd[0]);
    close(pipefd[1]);

    return 0;
}
