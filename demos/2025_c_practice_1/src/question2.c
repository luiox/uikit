// 基础字符串操作
// 1.	输入一个字符串（长度≤80），使用fgets安全读取。
// 2.	计算并输出字符串长度（不使用strlen）。
// 3.	统计数字字符个数。
// 4.	将字符串中的小写字母转换为大写（其他字符不变），并输出转换后的字符串。
// 5.	输入第二个字符串，比较两个字符串是否相等（不使用strcmp），如果不相等则输出第一个不同字符的位置。

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    // 1. 输入一个字符串（长度≤80），使用fgets安全读取。
    char str[100];
    printf("请输入一个字符串：\n");
    fgets(str, 80, stdin);

    // 2. 计算并输出字符串长度（不使用strlen）。
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    printf("字符串长度为：%d\n", length);

    // 3. 统计数字字符个数。
    int count = 0;
    for (int i = 0; i < length; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            count++;
        }
    }
    printf("数字字符个数为：%d\n", count);

    // 4. 将字符串中的小写字母转换为大写（其他字符不变），并输出转换后的字符串。
    char str2[100];
    for (int i = 0; i < length; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str2[i] = str[i] - 32;
        }
        else {
            str2[i] = str[i];
        }
    }
    str2[length] = '\0';
    printf("转换后的字符串为：%s\n", str2);

    // 5. 输入第二个字符串，比较两个字符串是否相等（不使用strcmp），如果不相等则输出第一个不同字符的位置。
    char str3[100];
    printf("请输入第二个字符串：");
    fgets(str3, 80, stdin);

    int len2 = strlen(str);
    int found_diff = 0;
    // 这里取大的，因为如果不一样的大小，那么在大的字符串中，一定包含小的字符串，按照大的一定能找到不同
    int max_len = (length > len2) ? length : len2;
    for (int i = 0; i < max_len; i++) {
        if (str[i] != str3[i]) {
            printf("第一个不同字符的位置为：%d\n", i);
            found_diff = 1;
            break;
        }
    }

    if (!found_diff) {
        printf("两个字符串相等\n");
    }

    return 0;
}
