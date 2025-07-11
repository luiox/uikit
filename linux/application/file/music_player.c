/****************************************************************************
 * @file MusicPlayer.c
 * @brief play music.
 * @details only support wav format file.
 * @author Canrad
 * @version 1.0
 * @date 2023/9/3
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>

#define BUFFER_SIZE 1024

int main(int argc, char** argv)
{
    if (2 != argc) {
        printf("Error!\nUsage: MusicPlayer filename\n");
        exit(-1);
    }
    int tarFd, srcFd;
    tarFd = open("/dev/dsp", O_WRONLY, 0666);
    if (-1 == tarFd) {
        perror("Open /dev/dsp file failed!");
        exit(-1);
    }

    // 设置声卡采样率为44100
    int rate = 44100;
    ioctl(tarFd, SNDCTL_DSP_SPEED, &rate);
    // 设置声卡为双声道
    int chanels = 2;
    ioctl(tarFd, SNDCTL_DSP_CHANNELS, &chanels);
    // 设置声卡采样模式为16
    int format = 16;
    ioctl(tarFd, SNDCTL_DSP_SETFMT, &format);

    srcFd = open(argv[1], O_RDONLY, 0666);
    if (-1 == srcFd) {
        perror("Read music file failed!");
        close(tarFd);
        exit(-1);
    }

    // 往声卡设备文件里面写数据
    int  count            = 0;
    char buf[BUFFER_SIZE] = {0};
    while (0 != count) {
        count = read(srcFd, buf, BUFFER_SIZE);
        if (count < 0) {
            perror("Read file failed!");
            exit(-1);
        }

        if (write(tarFd, buf, count) != count) {
            perror("Write file failed!");
            exit(-1);
        }
    }

    close(srcFd);
    close(tarFd);

    return 0;
}
