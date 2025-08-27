// 基础成绩统计
// 1.	输入5个学生的成绩（整数，范围0-100），如果输入无效则提示重新输入。
// 2.	计算并输出平均分（保留两位小数）。
// 3.	统计并输出及格（≥60）和不及格（<60）人数。
// 4.	将成绩从高到低排序（使用冒泡排序），并输出排序后的成绩。
// 5.	输入一个成绩值，使用二分查找法查找是否存在该成绩（存在输出位置，不存在提示）。

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main(int argc, char** argv)
{
    int score[5] = {0};
    // 当前获取了i个成绩
    int current_size = 0;
    // 1. 输入5个学生的成绩（整数，范围0-100），如果输入无效则提示重新输入。
    while (current_size < 5) {
        printf("请输入第%d个学生的成绩：", current_size + 1);
        scanf("%d", &score[current_size]);
        if (score[current_size] < 0 || score[current_size] > 100) {
            printf("输入无效，请重新输入！\n");
        }
        else {
            current_size++;
        }
    }
    // 输出下标和对应的成绩
    for (int i = 0; i < 5; i++) {
        printf("[%d]:%d\n", i, score[i]);
    }

    // 2. 输出平均成绩
    float sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += score[i];
    }
    printf("平均成绩为：%.2f\n", sum / 5);

    // 3. 统计并输出及格（≥60）和不及格（<60）人数。
    int pass = 0, fail = 0;
    for (int i = 0; i < 5; i++) {
        if (score[i] >= 60) {
            pass++;
        }
        else {
            fail++;
        }
    }
    printf("及格人数为：%d\n不及格人数为：%d\n", pass, fail);

    // 4. 将成绩从高到低排序（使用冒泡排序），并输出排序后的成绩。
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5 - i - 1; j++) {
            if (score[j] < score[j + 1]) {
                int temp = score[j];
                score[j] = score[j + 1];
                score[j + 1] = temp;
            }
        }
    }

    // 输出下标和对应的成绩
    for (int i = 0; i < 5; i++) {
        printf("[%d]:%d\n", i, score[i]);
    }

    // 5. 输入一个成绩值，使用二分查找法查找是否存在该成绩（存在输出位置，不存在提示）。
    int target = -1;
    while (1) {
        printf("请输入一个成绩值：");
        scanf("%d", &target);
        if (target < 0 || target > 100) {
            printf("输入的数字有误，请重新输入！\n");
        }
        else {
            break;
        }
    }

    // 二分查找
    int left = 0, right = 4;
    int found = 0, mid;
    while (left <= right) {
        mid = (left + right) / 2;
        if (score[mid] == target) {
            found = 1;
            break;
        }
        else if (score[mid] > target) {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }
    if (found == 1) {
        printf("下标为%d\n", mid);
    }
    else {
        printf("未找到\n");
    }

    return 0;
}
