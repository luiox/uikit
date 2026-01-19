#include <stdio.h>
#include <unistd.h>
#include <aio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

int main()
{
    // 1 准备缓冲区
    struct aiocb cb = {0};

    int fd = open("test.txt", O_RDONLY);
    if (-1 == fd) {
        printf("打开文件失败:%m\n"), exit(-1);
    }

    printf("打开文件成功!\n");
    // 2 异步操作
    cb.aio_fildes = fd;                             //文件描述符号
    cb.aio_buf    = (void*)malloc(BUFF_SIZE + 1);   //开内存
    memset(cb.aio_buf, 0, BUFF_SIZE + 1);           //清空  全部置零
    cb.aio_nbytes = BUFF_SIZE;                      //数据大小   单位是字节
    cb.aio_offset = 0;                              //文件中的偏移量
    // bzero

    int r = aio_read(&cb);
    if (-1 == r) {
        printf("异步读文件失败:%m\n"), close(fd), exit(-1);
    }

    printf("异步读文件成功!\n");

    // 3. 检查是否操作完毕
    int n = 0;
    while (0 != aio_error(&cb)) {   // aio_error返回值为0  循环结束
        n++;
    }

    // 4. 得到数据
    r = aio_return(&cb);
    if (r > 0) {
        printf("拿到数据:r:%d n:%d >> %s\n", r, n, cb.aio_buf);
    }

    // 5. 收尾
    free(cb.aio_buf);
    close(fd);
    return 0;
}