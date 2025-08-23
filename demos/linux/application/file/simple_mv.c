#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

bool isSameDirectory(const char* dir1, const char* dir2)
{
    assert(dir1);
    assert(dir2);

    int  size1 = strlen(dir1), size2 = strlen(dir2);
    int  n    = size1 > size2 ? size2 : size1;
    bool flag = true;
    for (int i = 0; i < n; i++) {
        if ('/' == dir1[i]) {
            if (dir1[i] != dir2[i]) {
                flag = false;
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char** argv)
{
    if (3 != argc) {
        printf("Error!\nUsagge: SimpleMV source_dirctory target_directory\n");
        exit(-1);
    }
    // 判断是否是同一个目录
    // bool isRename=isSameDirectory(argv[1], argv[2]);
    bool isRename = true;
    if (isRename) {   // 重命名
        int srcFd = open(argv[1], O_RDONLY);
        int tarFd = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
        // int tarFd=creat(argv[2],)
        long int n = lseek(srcFd, 0, SEEK_END);
        printf("file size = %ld\n", n);

        char* buf = (char*)malloc(n + 1);
        lseek(srcFd, 0, SEEK_SET);
        read(srcFd, buf, n);

        buf[n] = '\0';
        printf("%s\n", buf);

        write(tarFd, buf, n);
        close(srcFd);
        close(tarFd);
    }
    else {   // 剪切
    }


    return 0;
}
