#define _CRT_SECURE_NO_WARNINGS

/*
使用数组完成对学生信息的读取、
学生增删、姓名/学号/成绩的排序，
由学生姓名或学号进行查询学生全部信息。学生数目及科目数量固定。
*/
#include<stdio.h>//io,输入输出
#include<stdlib.h>
#include<string.h>

#define MAX_STUDENT 30


char stuName[MAX_STUDENT][20];//姓名
int stuID[MAX_STUDENT];//学号
int stuSocre[MAX_STUDENT];//成绩
int currentStu=-1;//目前学生信息的个数

//输出所有学生的信息
void printAllInfo()
{
	system("cls");
	for (int i = 0; i <= currentStu; i++)
	{
		printf("%d\t", stuID[i]);
		printf("%s\n", stuName[i]);
	}
}
//输出所有学生的成绩
void printAllSocre()
{
	system("cls");
	for (int i = 0; i <= currentStu; i++)
	{
		printf("%d\t", stuID[i]);
		printf("%d\n", stuSocre[i]);
	}
}
//录入
void studentScanf()
{
	printf("请输入学生学号,姓名,成绩");
	currentStu++;
	scanf("%d,%s,%d", &stuID[currentStu], &stuName[currentStu], &stuSocre[currentStu]);
}
//删除
void studentDelete()
{
	printf("请输入学号:");
	int id = 0;
	scanf("%d", &id);
	int a = 0;
	for (int i = 0; i <= currentStu; i++)//从前往后找
	{
		if (stuID[i] == id)//找到了记录下标到a里面
		{
			a = i;
		}
	}
	//删除这个信息
	for (int i = a; i < currentStu; i++)
	{
		stuID[i] = stuID[i + 1];
		stuSocre[i] = stuSocre[i + 1];
		strcpy(stuName[i], stuName[i + 1]);
	}
	currentStu--;
}
//查找
void studentSearchById()
{
	printf("请输入学号:");
	int id = 0;
	scanf("%d", &id);
	for (int i = 0; i <= currentStu; i++)
	{
		if (stuID[i] == id)
		{
			printf("%d\t", stuID[i]);
			printf("%s\n", stuName[i]);
			printf("%d\n", stuSocre[i]);
		}
	}
}
//排序
void studentSort()
{
	for (int j = currentStu; j >= 0; j--)
	{
		for (int i = 0; i < j; i++)
		{
			if (stuID[i]>stuID[i + 1])
			{
				int t1 = stuID[i]; stuID[i] = stuID[i + 1]; stuID[i + 1] = t1;
				int t2 = stuSocre[i]; stuSocre[i] = stuSocre[i + 1]; stuSocre[i + 1] = t2;
				char t3[20];
				strcpy(t3, stuName[i]);strcpy(stuName[i], stuName[i+1]);strcpy(stuName[i+1], t3);
			}
		}
	}

}


int main()
{
	while (1)
	{
		system("cls");
		printf("****************************\n");
		printf("1.输出所以学生信息\n");
		printf("2.输出所以学生成绩\n");
		printf("3.录入学生信息\n");
		printf("4.按照学号排序\n");
		printf("5.删除学生信息\n");
		printf("6.查找学生信息\n");
		printf("7.按学号排序\n");
		printf("10.退出\n");
		printf("****************************\n");

		printf("请输入你的选择:");
		int n=0;
		scanf("%d", &n);
		switch (n)
		{
		case 1:
			printAllInfo();
			break;
		case 2:
			printAllSocre();
			break;
		case 3:
			studentScanf();
			break;
		case 4:
			studentScanf();
			break;
		case 5:
			studentDelete();
			break;
		case 6:
			studentSearchById();
		case 7:
			studentSort();
			break;
		case 10:
			return 0;
			break;
		}
		system("pause");
	}


	//system("pause");
	return 0;
}
