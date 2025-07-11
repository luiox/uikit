/****************************************************************************
 * @file SimpleCP.c
 * @brief Implement a simple cp.
 * @details  Usage: SimpleCP source target
 * @author Canrad
 * @version 1.0
 * @date 2023/9/2
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// 默认一次读取4KB的文件
#define BUFFER_SIZE 4096

int main(int argc, char** argv)
{
    if (3 != argc) {
        printf("Error!\nUsage: SimpleCP source target\n");
        exit(-1);
    }

    // 打开源文件
    int srcFd = open(argv[1], O_RDONLY);
    if (-1 == srcFd) {
        printf("Open source file failed!\n");
        exit(-1);
    }
    // 打开目标文件，设置为可创建
    int tarFd = open(argv[2], O_WRONLY | O_CREAT, 0666);
    if (-1 == tarFd) {
        printf("Open target file failed!\n");
        exit(-1);
    }

    int size = -1;

    // 创建一个4KB的字符串缓冲区
    char buf[BUFFER_SIZE] = {0};

    // 循环把文件内4KB内容读入缓冲区
    while (0 != size) {
        size = read(srcFd, buf, BUFFER_SIZE);
        if (size < 0) {
            printf("Read file failed!\n");
            exit(-1);
        }
        // 把字符缓冲区内容写入目标文件，如果不等于size，那就是写入出错
        if (write(tarFd, buf, size) != size) {
            printf("Write file failed!\n");
            exit(-1);
        }
    }

    fsync(tarFd);   // 显示刷新缓冲区，写入磁盘

    // 关闭文件
    close(srcFd);
    close(tarFd);

    return 0;
}
