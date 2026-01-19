#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    // 描述符号集合
    fd_set fds;

    // 清空
    FD_ZERO(&fds);
    // 把标准输入添加到集合里去
    FD_SET(0, &fds);

    int  r;
    char buff[1024];
    while (1) {
		// 监视描述符号集合
        r = select(1, &fds, NULL, NULL, NULL);
        if (r > 0) {
			scanf("%s", buff);
            printf("%d有动静！\n", r);
            printf("接受到了:%s\n", buff);
        }
        else if (0 == r) {
            printf("没有动静!\n");
        }
        else {
            printf("出bug了!\n");
        }
    }

    return 0;
}
