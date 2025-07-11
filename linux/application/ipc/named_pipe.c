#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
    // 创建子进程
    pid_t pid = fork();

    // 4. 使用管道进行通信
    if (pid == 0) {
        // 子进程

        // 1. 打开管道文件
        int fd = open("my_pipe.pipe", O_RDONLY);

        char buf[256];
        // 这里就只读取三次
        for (int i = 0; i < 3; i++) {
            // 2. 使用管道文件
            // 从管道读取数据，并显示
            read(fd, buf, sizeof(buf));
            printf("子进程从管道读取到：%s\n", buf);
        }

        // 3. 关闭管道文件
        close(fd);
    }
    else if (pid > 0) {
        // 父进程

        // 1. 创建管道文件
        mkfifo("my_pipe.pipe", 0666);   // 除了执行权限都有

        // 2. 打开管道文件
        int fd = open("my_pipe.pipe", O_WRONLY);

        char buf[256];
        // 这里就只发三次
        for (int i = 0; i < 3; i++) {
            memset(buf, 0, sizeof(buf));
            // 键盘读取
            scanf("%s", buf);
            printf("父进程从键盘输入：%s\n", buf);
            // 3. 把数据写入管道文件
            write(fd, buf, sizeof(buf));
        }

        // 4. 关闭文件
        close(fd);

        // 5. 删除管道文件
        unlink("my_pipe.pipe");
    }
    else {
        perror("创建进程失败");
    }

    return 0;
}
