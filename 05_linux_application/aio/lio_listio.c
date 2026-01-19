#include <stdio.h>
#include <unistd.h>
#include <aio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

#define LIO_LISTIO_NUM 2

int main()
{
    //准备缓冲区
    struct aiocb rcb = {0};
    struct aiocb wcb = {0};

    //准备结构体指针数组
    struct aiocb* aiocb_list[LIO_LISTIO_NUM] = {0};

    int rfd = open("test.txt", O_RDONLY);
    if (-1 == rfd) {
        printf("打开文件失败:%m\n"), exit(-1);
    }

    printf("打开文件成功!\n");

    rcb.aio_fildes = rfd;                            //文件描述符号
    rcb.aio_buf    = (void*)malloc(BUFF_SIZE + 1);   //开内存
    memset(rcb.aio_buf, 0, BUFF_SIZE + 1);           //清空 全部置零
    rcb.aio_nbytes = BUFF_SIZE;                      //数据大小   单位是字节
    rcb.aio_offset = 0;                              //文件中的偏移量
    // bzero

    aiocb_list[0] = &rcb;   //结构体地址放到结构体指针数组内

    int r = aio_read(&rcb);
    if (-1 == r) {
        printf("异步读文件失败:%m\n"), close(rfd), exit(-1);
    }

    printf("异步读文件成功!\n");


    int wfd = open("test2.txt", O_WRONLY | O_APPEND);
    if (-1 == wfd) {
        printf("打开文件失败:%m\n"), exit(-1);
    }

    printf("打开文件成功!\n");
    // 2 异步操作
    wcb.aio_fildes = wfd;                            //文件描述符号
    wcb.aio_buf    = (void*)malloc(BUFF_SIZE + 1);   //开内存
    memset(wcb.aio_buf, 0, BUFF_SIZE + 1);           //清空  全部置零
    strcpy(wcb.aio_buf, "test");
    wcb.aio_nbytes = strlen("test");   //数据大小   单位是字节
    wcb.aio_offset = 0;                                                  //文件中的偏移量

    aiocb_list[1] = &wcb;   //结构体地址放到结构体指针数组内

    // bzero
    r = aio_write(&wcb);
    if (-1 == r) {
        printf("异步写文件失败:%m\n"), close(wfd), exit(-1);
    }

    printf("异步写文件成功!\n");

    // 4 用 lio_listio来监视多个io操作
    // r = lio_listio(LIO_NOWAIT,aiocb_list,2,NULL);//不阻塞直接返回
    r = lio_listio(LIO_WAIT, aiocb_list, 2, NULL);   //阻塞
    printf("lio_listio完毕 r:%d\n", r);

    // 5 得到数据
    r = aio_return(&rcb);
    if (r > 0) {
        printf("拿到数据:r:%d  >> %s\n", r, rcb.aio_buf);
    }

    r = aio_return(&wcb);
    if (r > 0) {
        printf("拿到数据:r:%d  >> %s\n", r, wcb.aio_buf);
    }

    free(wcb.aio_buf);
    free(rcb.aio_buf);
    close(rfd);
    close(wfd);


    return 0;
}
