#ifndef HEAD_H
#define	HEAD_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<conio.h>

#define MAX_ACT_SIZE  300//活动最大数量
#define MAX_TIME_SIZE 50//活动时间最大长度
#define MAX_REQUIRE_SIZE 200//活动要求最大长度

#define MAX_USERNAME_SIZE 16//账号最大长度
#define MAX_PASSWORD_SIZE 16//密码最大长度



//活动
typedef struct act
{
	int id;//活动ID
	char uesrname[20];//创建人名字
	char name[20];//活动名字
	char content[MAX_ACT_SIZE];//活动内容
	int num;//活动人数
	char time[50];//活动时间
	char requirements[MAX_REQUIRE_SIZE];//需求
	char personnelList[400];//人员名单
}ACT,LPACT;
//用户
typedef struct user
{
	char uesrname[MAX_USERNAME_SIZE];//账号
	char password[MAX_PASSWORD_SIZE];//密码
}USER;


#endif // !HEAD_H