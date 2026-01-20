// 学生信息处理
// 1.	定义一个学生结构体，包含学号（整型）、姓名（字符串，长度不超过20）、成绩（整型）。
// 2.	输入3个学生的信息（学号、姓名、成绩），成绩范围0-100，如果输入成绩不在范围内，提示重新输入。
// 3.	输出所有学生的信息，格式为：学号：xx，姓名：xx，成绩：xx。
// 4.	找出成绩最高的学生，并输出该学生的所有信息（包括学号、姓名、成绩）。
// 5.	将学生信息按照成绩从高到低排序（使用冒泡排序），并输出排序后的学生信息。

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>

// 1. 定义一个学生结构体，包含学号（整型）、姓名（字符串，长度不超过20）、成绩（整型）。
typedef struct student_struct
{
    // 学号
    int id;
    // 姓名
    char name[20];
    // 成绩
    int score;
} student_t;

student_t students[3];

int main(int argc, char** argv)
{
    // 2. 输入3个学生的信息（学号、姓名、成绩），成绩范围0-100，如果输入成绩不在范围内，提示重新输入。
    for (int i = 0; i < 3;) {
        printf("请输入第%d个学生的信息：\n", i + 1);
        printf("学号：");
        scanf("%d", &students[i].id);
        printf("姓名：");
        scanf("%s", students[i].name);
        printf("成绩：");
        scanf("%d", &students[i].score);
        if (students[i].score < 0 || students[i].score > 100) {
            printf("输入无效，请重新输入！\n");
        }
        else {
            i++;
        }
    }

    // 3. 输出所有学生的信息，格式为：学号：xx，姓名：xx，成绩：xx。
    printf("所有学生的信息为：\n");
    for (int i = 0; i < 3; i++) {
        printf("学号：%d，姓名：%s，成绩：%d\n", students[i].id, students[i].name, students[i].score);
    }

    // 4. 找出成绩最高的学生，并输出该学生的所有信息（包括学号、姓名、成绩）。
    int max_score = students[0].score;
    int max_index = 0;
    for (int i = 1; i < 3; i++) {
        if (students[i].score > max_score) {
            max_score = students[i].score;
            max_index = i;
        }
    }
    printf("最高成绩的学生是：学号：%d 姓名：%s 成绩：%d\n",
           students[max_index].id,
           students[max_index].name,
           students[max_index].score);

    // 5. 将学生信息按照成绩从高到低排序（使用冒泡排序），并输出排序后的学生信息。
    for (int i = 0; i < 3 - 1; i++) {
        for (int j = 0; j < 3 - i - 1; j++) {
            if (students[j].score < students[j + 1].score) {
                student_t temp;
                // 结构体不能直接利用temp = students[j]复制，应该使用memcpy或者一个一个成员复制
                memcpy(&temp, &students[j], sizeof(student_t));
                memcpy(&students[j], &students[j + 1], sizeof(student_t));
                memcpy(&students[j + 1], &temp, sizeof(student_t));
            }
        }
    }
    printf("排序后：\n");
    for (int i = 0; i < 3; i++) {
        printf("%s %d\n", students[i].name, students[i].score);
    }

    return 0;
}
